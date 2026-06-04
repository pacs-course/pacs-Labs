// This example is modified starting from a MIT licensed NVIDIA one, 
// Original license follows.


/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

//! \file
//! \brief Hour-2 solution: extend the MPI solver with C++ host threads.
//!
//! This version starts from solution/01_mpi.cpp and runs the prev, next, and
//! inner subdomain updates concurrently inside each MPI rank.

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <vector>
#include <ranges>
#include <algorithm> 
#include <numeric>
#include <execution>
#include <thread>
#include <atomic>
#include <barrier>

/// Physical, numerical, and MPI-decomposition parameters.
struct parameters {
  double dx, dt;              ///< Space and time steps.
  long nx, ny, ni;            ///< Local x-size, y-size, and number of steps.
  int rank = 0, nranks = 1;   ///< MPI rank and communicator size.

  static constexpr double alpha() { return 1.0; } // Thermal diffusivity

  /// Build parameters from: executable nx ny ni.
  parameters(int argc, char *argv[]);
    
  long nit() { return ni; }
  long nout() { return 1000; }
  long nx_global() { return nx * nranks; }
  long ny_global() { return ny; }
  double gamma() { return alpha() * dt / (dx * dx); }
  long n() { return ny * (nx + 2 /* two halo x-rows */); }
};

/// Half-open 2D iteration box: [x_begin, x_end) x [y_begin, y_end).
struct grid {
  long x_begin, x_end;
  long y_begin, y_end;
};

/// Apply the stencil on a subdomain and return its energy contribution.
double apply_stencil(double* u_new, double* u_old, grid g, parameters p);
/// Initialize the two time levels.
void initial_condition(double* u_new, double* u_old, long n);

/// Evolve the interior rows, which do not need halo data from other ranks.
double inner(double* u_new, double* u_old, parameters p);
/// Exchange with rank - 1 when present, then evolve the first physical x-row.
double prev (double* u_new, double* u_old, parameters p);
/// Exchange with rank + 1 when present, then evolve the last physical x-row.
double next (double* u_new, double* u_old, parameters p);

int main(int argc, char *argv[]) {
  // Parse CLI parameters
  parameters p(argc, argv);

  // Hour 2 may call MPI concurrently from prev, next, and inner threads.
  // Request full MPI thread support and fail early if the implementation
  // cannot provide it.
  int mt;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mt);
  if (mt != MPI_THREAD_MULTIPLE) {
      std::cerr << "MPI cannot be called from multiple host threads" << std::endl;
      std::terminate();
  }

  MPI_Comm_size(MPI_COMM_WORLD, &p.nranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &p.rank);

  // Allocate the two time levels, including halo rows.
  std::vector<double> u_new(p.n()), u_old(p.n());

  initial_condition(u_new.data(), u_old.data(), p.n());

  using clk_t = std::chrono::steady_clock;
  auto start = clk_t::now();
      
  // All three worker threads add their local energy to this shared accumulator.
  std::atomic<double> energy = 0.;

  // Two waits per iteration split the step into:
  // 1. compute all three subdomains;
  // 2. let the inner thread reduce, print, reset, and swap before the other
  //    threads swap their private pointer captures and start the next step.
  std::barrier bar(3);
  


  // Boundary task depending on rank - 1.
  std::thread thread_prev([p, u_new = u_new.data(), u_old = u_old.data(), 
                             &energy, &bar]() mutable {
        for (long it = 0; it < p.nit(); ++it) {
            energy += prev(u_new, u_old, p);
            
            // End of compute phase: wait until next and inner are done.
            bar.arrive_and_wait();
            
            // Wait until inner has reduced/reset/swapped.
            bar.arrive_and_wait();
            
            // Each lambda owns pointer copies, so each thread must swap them.
            std::swap(u_new, u_old);
        }
    });
    
 
  // Boundary task depending on rank + 1.
  std::thread thread_next([p, u_new = u_new.data(), u_old = u_old.data(), 
                             &energy, &bar]() mutable {
        for (long it = 0; it < p.nit(); ++it) {
            energy += next(u_new, u_old, p);

            // End of compute phase: wait until prev and inner are done.
            bar.arrive_and_wait();

            // Wait until inner has reduced/reset/swapped.
            bar.arrive_and_wait();

            // Each lambda owns pointer copies, so each thread must swap them.
            std::swap(u_new, u_old);
        }
    });
    

  // Interior task. It also owns the serial phase between the two barriers.
  std::thread thread_inner([p, u_new = u_new.data(), u_old = u_old.data(), 
                              &energy, &bar]() mutable {
      for (long it = 0; it < p.nit(); ++it) {
          energy += inner(u_new, u_old, p);

          // End of compute phase: all energy contributions are now present.
          bar.arrive_and_wait();
      
          // Only this thread performs the per-step rank reduction and output.
          MPI_Reduce(p.rank == 0 ? MPI_IN_PLACE : &energy, &energy, 1, MPI_DOUBLE, MPI_SUM, 0,
                     MPI_COMM_WORLD);

          if (p.rank == 0 && it % p.nout() == 0) {
              std::cerr << "E(t=" << it * p.dt << ") = " << energy << std::endl;
          }

          std::swap(u_new, u_old);
          energy = 0;
      
          // Release boundary threads into their local pointer swaps.
          bar.arrive_and_wait();
      }
  });
  
  thread_prev.join();
  thread_next.join();
  thread_inner.join();
  

  auto time = std::chrono::duration<double>(clk_t::now() - start).count();

  auto grid_size = static_cast<double>(p.nx * p.ny * sizeof(double) * 2) * 1e-9; // GB

  auto memory_bw = grid_size * static_cast<double>(p.nit()) / time; // GB/s

  if (p.rank == 0) {
      std::cerr << "Rank " << p.rank << ": local domain " << p.nx << "x" << p.ny << " (" << grid_size << " GB): " 
                << memory_bw << " GB/s" << std::endl;

      std::cerr << "All ranks: global domain " << p.nx_global() << "x" << p.ny_global() << " (" 
                << (grid_size * p.nranks) << " GB): " << memory_bw * p.nranks << " GB/s" << std::endl; 
  }


  // Write one binary file: header, final time, then one contiguous field block
  // per rank. The field skips the lower halo row with u_new.data() + p.ny.
  
  MPI_File f;

  MPI_File_open(MPI_COMM_WORLD, "output_02", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &f);

  auto header_bytes = 2 * sizeof(long) + sizeof(double);

  auto values_per_rank = p.nx * p.ny;

  auto values_bytes_per_rank = values_per_rank * sizeof(double);

  MPI_File_set_size(f, header_bytes + values_bytes_per_rank * p.nranks);

  MPI_Request req[3] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};

  if (p.rank == 0) {
      long total[2] = {p.nx * p.nranks, p.ny};

      double time = p.nit() * p.dt;

      MPI_File_iwrite_at(f, 0, total, 2, MPI_UINT64_T, &req[1]);

      MPI_File_iwrite_at(f, 2 * sizeof(long), &time, 1, MPI_DOUBLE, &req[2]);
  }

  auto values_offset = header_bytes + p.rank * values_bytes_per_rank;

  MPI_File_iwrite_at(f, values_offset, u_new.data() + p.ny, values_per_rank, MPI_DOUBLE, &req[0]);

  MPI_Waitall(p.rank == 0 ? 3 : 1, req, MPI_STATUSES_IGNORE);

  MPI_File_close(&f);


  MPI_Finalize();
  return 0;
}

/// Read local problem size and number of iterations from the command line.
parameters::parameters(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "ERROR: incorrect arguments" << std::endl;
    std::cerr << "  " << argv[0] << " <nx> <ny> <ni>" << std::endl;
    std::terminate();
  }
  nx = std::stoll(argv[1]);
  ny = std::stoll(argv[2]);
  ni = std::stoll(argv[3]);
  
  dx = 1.0 / nx;
  dt = dx * dx / (5. * alpha());
}

/// Update one physical grid point and return its contribution to total energy.
double stencil(double *u_new, double *u_old, long x, long y, parameters p) {
  auto idx = [=](auto x, auto y) { 
      // Index local storage, including halo rows, in row-major order.
      assert(x >= 0 && x < 2 * p.nx);
      assert(y >= 0 && y < p.ny);
      return x * p.ny + y;
  };

  // Apply physical boundary conditions. In x, only the ranks at the two global
  // ends own physical boundaries; interior x-neighbors are supplied by halos.
  if (y == 1) {
    u_old[idx(x, y - 1)] = 0;
  }
  if (y == (p.ny - 2)) {
    u_old[idx(x, y + 1)] = 0;
  }
  if (p.rank == 0 && x == 1) {
    u_old[idx(x - 1, y)] = 1;
  }
  if (p.rank == (p.nranks - 1) && x == p.nx) {
    u_old[idx(x + 1, y)] = 0;
  }

  u_new[idx(x, y)] = (1. - 4. * p.gamma()) * u_old[idx(x, y)] +
                     p.gamma() * (u_old[idx(x + 1, y)] + u_old[idx(x - 1, y)] +
                                  u_old[idx(x, y + 1)] + u_old[idx(x, y - 1)]);

  return u_new[idx(x, y)] * p.dx * p.dx;
}

/// Apply the stencil on a subdomain and return its energy contribution.
double apply_stencil(double* u_new, double* u_old, grid g, parameters p) {
  auto xs = std::views::iota(g.x_begin, g.x_end);
  auto ys = std::views::iota(g.y_begin, g.y_end);
  auto ids = std::views::cartesian_product(xs, ys);
  
  return std::transform_reduce(
    std::execution::par, ids.begin(), ids.end(), 
    0., std::plus{}, [u_new, u_old, p](auto idx) {
      auto [x, y] = idx;
      return stencil(u_new, u_old, x, y, p);
  });
}

/// Initialize the two time levels.
void initial_condition(double* u_new, double* u_old, long n) {
  std::fill_n(std::execution::par, u_old, n, 0.0);
  std::fill_n(std::execution::par, u_new, n, 0.0);
}

/// Evolve the interior rows, which do not depend on neighboring ranks.
double inner(double *u_new, double *u_old, parameters p) {
  grid g{.x_begin = 2, .x_end = p.nx, .y_begin = 1, .y_end = p.ny - 1};
  return apply_stencil(u_new, u_old, g, p);
}

/// Exchange with rank - 1, if present, and update the first physical x-row.
double prev(double *u_new, double *u_old, parameters p) {
  if (p.rank > 0) {
    // Send this rank's first physical x-row to rank - 1.
    MPI_Send(u_old + p.ny, p.ny, MPI_DOUBLE, p.rank - 1, 0, MPI_COMM_WORLD);
    // Receive the lower halo x-row from rank - 1.
    MPI_Recv(u_old + 0, p.ny, MPI_DOUBLE, p.rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  
  grid g{.x_begin = 1, .x_end = 2, .y_begin= 1, .y_end = p.ny - 1};
  return apply_stencil(u_new, u_old, g, p);
}

/// Exchange with rank + 1, if present, and update the last physical x-row.
double next(double *u_new, double *u_old, parameters p) {
  if (p.rank < p.nranks - 1) {
    // Receive the upper halo x-row from rank + 1.
    MPI_Recv(u_old + (p.nx + 1) * p.ny, p.ny, MPI_DOUBLE, p.rank + 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    // Send this rank's last physical x-row to rank + 1.
    MPI_Send(u_old + p.nx * p.ny, p.ny, MPI_DOUBLE, p.rank + 1, 1, MPI_COMM_WORLD);
  }
  
  grid g{.x_begin = p.nx, .x_end = p.nx + 1, .y_begin = 1, .y_end = p.ny - 1};
  return apply_stencil(u_new, u_old, g, p);
}
