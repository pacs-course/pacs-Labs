#include <chrono>
#include <cstring>  // For memcpy.
#include <functional>
#include <iostream>
#include <random>
#include <vector>

// The cache size of your processor, in bytes. Adjust accordingly.
// Use the command `getconf -a | grep "LEVEL1_DCACHE_SIZE"`
const size_t cache_size = 8 * 1024 * 1024;  // 8 * KB * MB

// The number of integers that fit in the CPU cache,
// which is useful for picking a sample set size.
const size_t ints_in_cache = cache_size / sizeof(int);

// Used to avoid typing "high_resolution_clock" repeatedly.
// Note that the high-resolution clock is not guaranteed to be monotonic,
// but this shouldn't be a problem as long as you don't change your OS clock
// while running this!
using timer = std::chrono::high_resolution_clock;

// Invalidates the entire CPU cache so that it has minimal impact on
// our timings.
void clear_cache() {
  // A dumb but effective way to clear the cache is to copy
  // at least as much memory as there is cache.
  static int buffer_A[2 * cache_size];
  static int buffer_B[2 * cache_size];

  memcpy(buffer_A, buffer_B, cache_size);
}

// Populates each integer in the given data set
// using the given random number generator.
template <class R>
void populate_dataset(std::vector<int> &data, R randi) {
  for (int &d : data) d = randi();
}

long int compute(const std::vector<int> &data) {
  long int sum = 0;

  // Square each value.
  for (int d : data) sum += d * d;

  return sum / data.size();
}

long int compute_pointer(const std::vector<int *> &data) {
  long int sum = 0;

  // Square each value.
  for (auto d : data) sum += (*d) * (*d);

  return sum / data.size();
}

void test(unsigned int iterations, std::vector<int> &data,
          const std::function<long int()> &compute) {
  // Seed the RNG with actual hardware/OS randomness from random_device.
  std::default_random_engine engine(std::random_device{}());

  // Since the "work" we are doing is squaring each integer,
  // initialize them with some value between 1 and 10.
  std::uniform_int_distribution<int> rand_int(1, 10);

  // Bundle this all up into a closure that populate_dataset can call:
  auto randi = [&] { return rand_int(engine); };

  // Used to sum how much time all of our runs took,
  // excluding the setup and measurement work we do around them.
  timer::duration time_total{0};

  for (unsigned int i = 0; i < iterations; ++i) {
    populate_dataset(data, randi);
    clear_cache();

    const auto start = timer::now();
    const int result = compute();
    const auto end = timer::now();

    time_total += (end - start);

    // We write out the result to make sure the compiler doesn't
    // optimize out the result as a dead store, and to give us something
    // to look at.
    std::cout << "Run " << i + 1 << ", result = " << result << "\r";
    std::cout.flush();
  }
  std::cout << std::endl << std::endl;

  {
    using namespace std::chrono;
    const auto time_total_ms = duration_cast<milliseconds>(time_total).count();

    const auto time_average =
        duration_cast<microseconds>(time_total).count() / iterations;

    std::cout << iterations << " runs took " << time_total_ms / 1000 << "."
              << time_total_ms % 1000 << " seconds total," << std::endl;

    std::cout << "Averaging " << time_average / 1000 << "."
              << time_average % 1000 << " milliseconds per run" << std::endl;
  }
  std::cout << "-------------------------------------------------" << std::endl;
}

int main(int argc, char **argv) {
  const unsigned int iterations = 10;  // Number of tests to run.

  // 1. Our test data set.
  auto data = std::vector<int>(ints_in_cache * 10);

  // 2. Provide pointer accessor.
  auto pointers = std::vector<int *>(data.size());
  for (size_t i = 0; i < data.size(); ++i) pointers[i] = &data[i];

  // 3. Simulate random access
  auto shuffle_pointers = pointers;
  std::shuffle(shuffle_pointers.begin(), shuffle_pointers.end(),
               std::default_random_engine{});

  test(iterations, data, [&]() { return compute(data); });
  test(iterations, data, [&]() { return compute_pointer(pointers); });
  test(iterations, data, [&]() { return compute_pointer(shuffle_pointers); });

  return 0;
}
