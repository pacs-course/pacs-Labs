#include <omp.h>

#include <iostream>

/**
 * @file 01-hello_world_simple.cpp
 * @brief Minimal introduction to an OpenMP parallel region.
 *
 * This example contrasts a normal serial print with a print executed inside an
 * OpenMP `parallel` region. When the program enters the parallel region, the
 * runtime creates a team of threads and each thread executes the same block.
 *
 * Expected output:
 * - the first message is printed exactly once;
 * - the second message is printed once per thread;
 * - the order of the parallel messages is not guaranteed.
 */

/**
 * @brief Run the simplest possible OpenMP example.
 */
int
main()
{
  /// Serial code: executed only by the original thread.
  std::cout << "Hello World!" << std::endl;

  /**
   * @brief Start a parallel region.
   *
   * Every thread in the team executes the following block. The output may
   * appear interleaved because `std::cout` is shared by all threads.
   */
#pragma omp parallel
  {
    std::cout << "Hello World! (This time in parallel!)" << std::endl;
  }

  return 0;
}
