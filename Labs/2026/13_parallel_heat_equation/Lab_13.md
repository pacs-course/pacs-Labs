---
marp: true
size: 16:9
style: |
    img[alt~="center"] {
      display: block;
      margin: 0 auto;
    }
---
# Laboratory 13
## MPI, C++ concurrency, and sender/receiver execution

### Paolo Joseph Baioni
### 05/06/2026

---
## Outline
1. MPI-based heat-equation solver
2. Adding C++ standard-library concurrency
3. Refactoring with [P2300 `std::execution`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r10.html)

---

# Prerequisites

For part 3 we use NVIDIA's [`stdexec`](https://github.com/NVIDIA/stdexec) implementation. It is included as a submodule: if you did not clone this repository with `--recursive`, the `stdexec` directory may be empty.

In that case, run:
```bash
git submodule update --init --recursive
```
Alternatively, clone `stdexec` manually:
```bash
git clone --depth=1 https://github.com/NVIDIA/stdexec.git
```

---

# Prerequisites

We use C++ execution policies: the implementation may assume that their requirements are satisfied.

In particular, 
 - user code passed to `std::execution::par` must not introduce data races
 - user code passed to `std::execution::par_unseq` must not rely on inter-iteration dependencies or blocking synchronization

 >  par: Threads $\rightarrow$ must be data-race free $\rightarrow$ may wait for messages 
 >  par_unseq: SIMD Lanes $\rightarrow$ must be independent $\rightarrow$ no message exchange

See [2026/03-functions-algorithms](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/03-functions-algorithms).

---
# Note on toolchains

The solution `Makefile` defaults to the GNU toolchain.

With Clang/Open MPI, you might use Clang as the backend compiler while keeping `libstdc++`, for example:

```bash
OMPI_CXX=clang++ mpicxx -stdlib=libstdc++ -std=c++23 \
  -I../stdexec/include -O3 -ffast-math -march=native -DNDEBUG \
  -o 03_senders_and_receivers 03_senders_and_receivers.cpp -ltbb
```
Using `-stdlib=libc++` may fail because the required C++23 ranges and parallel algorithm facilities may not yet be available in that standard library.

---

# Application: 2D heat equation

We solve the 2D heat equation using a finite-difference discretization on a rectangular domain.

Each MPI rank owns a slice of the domain plus two halo layers.

![bg right 100%](img/bc+ic.png)

---

# Grid and stencil

Each time step updates one point from its four direct neighbors:

$$
T_{i,j}^{n+1} = (1 - 4\gamma)T_{i,j}^{n}
 + \gamma(T_{i+1,j}^{n} + T_{i-1,j}^{n}
 + T_{i,j+1}^{n} + T_{i,j-1}^{n})
$$

![bg left 75%](img/grid.png)

---

# Halo exchange

Before updating boundary cells, ranks exchange halo data with their neighbors.

![w:900 center](img/halo_exchange.png)

---

# 1. MPI-based heat-equation solver
## From `exercise.cpp` to `01_mpi.cpp`

Complete:

- C++ ranges and parallel algorithms for the stencil loop
- MPI initialization and rank discovery
- blocking halo exchange with `MPI_Send` and `MPI_Recv`
- global energy reduction with `MPI_Reduce`
- final parallel file output with MPI-IO

---

# Parallel I/O

We write one binary output file:

- global dimensions
- final physical time
- one contiguous field block per rank

Relevant MPI routines:

- [`MPI_File_open`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_File_open.3.html)
- [`MPI_File_set_size`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_File_set_size.3.html)
- [`MPI_File_iwrite_at`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_File_iwrite_at.3.html)
- [`MPI_Waitall`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_Waitall.3.html)

---

# Visualizing the result

Install the plotting dependencies once, if needed:

```bash
python -m venv ~/my_env
source ~/my_env/bin/activate
pip install --upgrade pip
pip install numpy matplotlib
```
Then plot any binary output from the `solution` directory:

```bash
make plot OUTPUT=output_01
```

(The default is `OUTPUT=output`)

---
# Visualizing the result

$$
U(t) = \int_\Omega \rho c T(x,y) d\Omega
$$

should converge towards a finite value (steady state)
![bg left 100%](img/output.png)

---

# 2. Adding C++ standard-library concurrency
## From `01_mpi.cpp` to `02_mpi+concurrency.cpp`

The three local updates are independent after their required halo communication:

- `prev`
- `next`
- `inner`

We assign them to three host threads.

---

# MPI and host threads

Plain `MPI_Init` is no longer enough once multiple host threads may call MPI.

Use:

```cpp
MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mt);
```

Then check that the provided support level is [`MPI_THREAD_MULTIPLE`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_Init_thread.3.html).

---

# C++ concurrency tools

We add:

- [`std::thread`](https://www.cppreference.com/w/cpp/thread/thread.html) for the three concurrent tasks
- [`std::atomic<double>`](https://en.cppreference.com/w/cpp/atomic/atomic) for the shared energy accumulator
- [`std::barrier`](https://en.cppreference.com/w/cpp/thread/barrier.html) to separate compute, reduction, reset, and pointer swap phases

Keep the levels separate:

- [`MPI_Barrier`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_Barrier.3.html) synchronizes ranks. 
- [`std::barrier`](https://en.cppreference.com/w/cpp/thread/barrier.html) synchronizes threads inside one process.

---

# 3. Refactoring with sender/receiver execution (P2300)
## From `02_mpi+concurrency.cpp` to `03_senders_and_receivers.cpp`

Now we keep the same dependency graph:

```text
prev   \
next    > when_all -> reduce -> print -> swap
inner  /
```

The code moves from manual threads and barriers to composable execution objects.

---

# Sender/receiver idea

Simplifying, schedulers, senders, and receivers can be read as standard-library abstractions of thread pools, futures, and promises.

A sender is a description of deferred work: it describes an asynchronous operation and the values, errors, or completion signal it may produce.

A scheduler says where work should run.

> Nothing starts just because a sender object exists: work starts only when the sender is connected to a receiver, which provides the destination for the result, the error, or the stopped signal. This is one safety aspect of the model: completion is part of the protocol, so a started operation has an explicit place where its value, error, or cancellation signal must go.

---

# Composing senders

Sender adaptors are pipeable:

```cpp
snd | ex::then(f) | ex::then(g)
```

This reads like a Unix pipeline and builds the nested composition `then(then(snd, f), g)`.

The important point is the work graph: values flow from `snd`, then through `f`, then through `g`.

`then` attaches a continuation.

---

# Building a work graph

Senders can be chained together to describe a task graph before running it.

Useful building blocks:

- `schedule(scheduler)` creates a sender that completes on that scheduler when started
- `then(sender, f)` attaches a continuation
- `split(sender)` can share one sender result with more than one continuation
- `when_all(a, b, c)` joins independent senders and completes when all have completed
- `sync_wait(sender)` is a sender consumer: it waits at the edge of the graph

---

# Why this?

This is not yet a widely available portable production baseline.

It is a working implementation of ideas behind proposed and incoming standard execution facilities, useful to understand where C++ concurrency is heading.

Reference:

[P2300 `std::execution`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r10.html)

> expresses dependencies instead of manually coordinating threads

---

# Extra: GPU offloading

The approach shown in the code can also serve as a starting point to investigate GPU offload with `stdpar` and `nvc++`.

Reference on NVIDIA documentation:

[`NVC++ and C++ Standard Parallelism`](https://docs.nvidia.com/hpc-sdk/compilers/hpc-compilers-user-guide/index.html#stdpar-cpp)

The three host threads do not imply three GPUs: MPI ranks, host threads, and GPU execution are independent layers of parallelism.

> MPI ranks distribute the domain across processes/nodes. 
> C++ threads split local work inside one rank: `prev`, `next`, `inner`. 
> `stdpar/nvc++` may offload `std::execution::par` algorithms to a GPU.
> GPU mapping is a deployment/runtime choice.

---

# Appendix: stdexec calls in `03`

| API | What it is / what it does |
| --- | --- |
| `exec::static_thread_pool ctx{3}` | Creates a fixed-size pool of three worker threads. |
| `ctx.get_scheduler()` | Returns a scheduler associated with the thread pool. |
| `stde::scheduler auto` | Constrains a parameter or variable to model the scheduler concept. |
| `stde::sender auto` | Constrains a return value to model the sender concept. |


---

# Appendix: stdexec calls in `03`

| API | What it is / what it does |
| --- | --- |
| `stde::just()` | Creates a sender that, when started, completes immediately and produces no value. |
| `stde::then(f)` | Adaptor that runs `f` after the previous sender completes. |
| `stde::on(sch, snd)` | Adaptor that arranges execution of `snd` on scheduler `sch`. |
| `stde::when_all(a, b, c)` | Joins independent senders; completes after all of them complete. |
| `stde::sync_wait(snd)` | Starts the sender graph and blocks until it completes. |

---


# References

- Open MPI: [`MPI_Init_thread`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_Init_thread.3.html), [`MPI_Barrier`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_Barrier.3.html)
- Open MPI MPI-IO: [`MPI_File_open`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_File_open.3.html), [`MPI_File_set_size`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_File_set_size.3.html), [`MPI_File_iwrite_at`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_File_iwrite_at.3.html), [`MPI_Waitall`](https://docs.open-mpi.org/en/main/man-openmpi/man3/MPI_Waitall.3.html)
- C++ reference: [`<execution>`](https://en.cppreference.com/cpp/header/execution), [`std::execution::par`](https://en.cppreference.com/cpp/algorithm/execution_policy_tag), [`std::thread`](https://www.cppreference.com/w/cpp/thread/thread.html), [`std::atomic`](https://en.cppreference.com/w/cpp/atomic/atomic), [`std::barrier`](https://en.cppreference.com/w/cpp/thread/barrier.html)
- P2300 proposal: [`std::execution`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r10.html)
- NVIDIA [`stdexec` implementation](https://github.com/NVIDIA/stdexec) and [documentation](https://nvidia.github.io/stdexec/index.html)
- David Olsen, CppCon 2022: [Standard Parallelism: We Need On-Ramps](https://meetingcpp.com/mcpp/slides/2022/Olsen-MeetingCpp-Cpp_Std_Parallelism9578.pdf)
- NVIDIA [`nvc++` standard parallelism](https://docs.nvidia.com/hpc-sdk/compilers/hpc-compilers-user-guide/index.html#stdpar-cpp)
- Jonas Latt, GTC 2021: [Porting a scientific application to GPU using C++](https://www.researchgate.net/publication/356217103_Porting_a_scientific_application_to_GPU_using_C_standard_parallelism)
