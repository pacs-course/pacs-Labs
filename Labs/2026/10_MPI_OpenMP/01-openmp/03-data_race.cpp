#include <omp.h>

#include <iostream>

/**
 * @file 03-data_race.cpp
 * @brief Show why not every loop can be parallelized safely.
 *
 * The program performs the same array update twice:
 * - once in serial, which is the reference behavior;
 * - once with OpenMP, where the loop is intentionally parallelized even
 *   though iterations depend on each other.
 *
 * The final checksums compare the serial and parallel outcomes.
 */

/**
 * @brief Compare a serial loop with an unsafe parallel version.
 */
int
main()
{
  constexpr size_t N = 5;

  int a[N];
  int b[N];

  /**
   * @brief Initialize the arrays before the serial run.
   *
   * After this loop, `a[i] = i` and `b[i] = 2` for every valid index.
   */
  for (size_t i = 0; i < N; ++i)
    {
      a[i] = i;
      b[i] = 2 * 1;
    }

  std::cout << "Serial:" << std::endl;
  /// Sum of the values produced by the serial reference computation.
  int serial_checksum = 0;

  /**
   * @brief Serial reference loop.
   *
   * Each iteration reads `a[i + 1]`: this means the loop has a 
   * loop-carried dependency and iteration order matters.
   */
  for (size_t i = 0; i < N - 1; ++i)
    {
      a[i] = a[i + 1] + b[i];

      std::cout << "i = " << i << ", a[" << i << "] = a[" << i + 1
                << "] + b[" << i << "] = " << a[i + 1] << " + "
                << b[i] << " = " << a[i] << std::endl;

      serial_checksum += a[i];
    }

  /**
   * @brief Reinitialize the arrays before the parallel run.
   */
  for (size_t i = 0; i < N; ++i)
    {
      a[i] = i;
      b[i] = 2 * 1;
    }

  int n_threads;
  /**
   * @brief Intentionally unsafe OpenMP version.
   *
   * The `for` directive distributes loop iterations across threads, but this
   * loop is not independent: iteration `i` reads memory that iteration `i + 1`
   * may overwrite. The program is therefore incorrect in parallel even if the
   * output printing itself is protected.
   */
#pragma omp parallel shared(a, b, n_threads)
  {
#pragma omp master
    {
      n_threads = omp_get_num_threads();
      std::cout << std::endl
                << "Parallel (number of threads: " << n_threads
                << "):" << std::endl;
    }
#pragma omp barrier

#pragma omp for
    for (size_t i = 0; i < N - 1; ++i)
      {
        /**
         * @warning This is the key bug in the example.
         *
         * If another thread updates `a[i + 1]` before the current thread reads
         * it, the computed value of `a[i]` changes. The loop body is therefore
         * not safe to parallelize with a plain `omp for`.
         */
        a[i] = a[i + 1] + b[i];

#pragma omp critical
        {
          std::cout << "i = " << i << ", a[" << i << "] = a[" << i + 1
                    << "] + b[" << i << "] = " << a[i + 1] << " + "
                    << b[i] << " = " << a[i] << std::endl;
        }
      }
  }

  /// Reduction is safe here because summing `a[i]` is independent across `i`.
  int parallel_checksum = 0;
#pragma omp parallel for reduction(+ : parallel_checksum)
  for (size_t i = 0; i < N - 1; ++i)
    {
      parallel_checksum += a[i];
    }

  std::cout << std::endl
            << "Serial   result: " << serial_checksum << std::endl
            << "Parallel result: " << parallel_checksum << std::endl;
}
