#include <omp.h>

#include <iostream>

/**
 * @file 02-hello_world_advanced.cpp
 * @brief Demonstrate basic OpenMP thread-management directives.
 *
 * This example introduces:
 * - `parallel` to create a team of threads;
 * - `private` to give each thread its own variable instance;
 * - `critical` to serialize access to a code block;
 * - `barrier` to synchronize all threads;
 * - `master` to let only thread 0 execute a block.
 *
 * This file extends the first "hello world" example with the most common
 * building blocks of OpenMP coordination.
 */

/**
 * @brief Print a message from each thread and inspect the team size.
 */
int
main()
{
  /// Total number of threads in the current OpenMP team.
  int n_threads;
  /// Identifier of the current thread inside the team.
  int thread_id;

  /**
   * @brief Start (fork) a team of threads.
   *
   * `thread_id` is declared `private`, so each thread owns an independent copy
   * of that variable. Without `private`, all threads would share the same
   * memory location and the printed identifiers would be unreliable.
   */
#pragma omp parallel private(thread_id)
  {
    /// OpenMP thread IDs go from `0` to `omp_get_num_threads() - 1`.
    thread_id = omp_get_thread_num();

    /**
     * @brief Protected output section.
     *
     * Only one thread at a time may execute a `critical` region. This avoids
     * mixed or interleaved output on the terminal.
     *
     * Related directives worth comparing:
     * - `single`: one thread executes the block, not necessarily thread 0;
     * - `master`: only thread 0 executes the block;
     * - `atomic`: protects one simple memory update.
     */
#pragma omp critical
    std::cout << "Hello World from thread " << thread_id << "!"
              << std::endl;


    /// Synchronize the whole team before continuing.
#pragma omp barrier

    /**
     * @brief Query the number of threads once all threads have arrived here.
     *
     * The `master` directive runs the block only on thread 0. Unlike `single`,
     * it does not imply any extra synchronization at the end.
     */
#pragma omp master
    {
      n_threads = omp_get_num_threads();
      std::cout << "Number of threads = " << n_threads << std::endl;
    }
  }

  /// At the end of the parallel region, all worker threads are joined.

  return 0;
}
