#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

inline static constexpr size_t vec_size = 1000;

int lookup(const size_t &index) {
  // Generate an arbitrary dataset.
  auto data = std::vector<int>(vec_size);
  std::iota(data.begin(), data.end(), 0);

  return data[index];
}

int lookup_static(const size_t &index) {
  // Generate an arbitrary dataset.
  static std::vector<int> data;

  if (data.empty()) {
    data.resize(vec_size);
    std::iota(data.begin(), data.end(), 0);
  }

  return data[index];
}

using timer = std::chrono::high_resolution_clock;

int main(int argc, char **argv) {
  const size_t n_iterations = 1e7;

  std::default_random_engine engine(std::random_device{}());
  std::uniform_int_distribution<int> rand_int(0, vec_size - 1);

  using namespace std::chrono;
  // standard lookup
  {
    const auto start = timer::now();
    for (size_t i = 0; i < n_iterations; ++i) {
      lookup(rand_int(engine));
    }
    const auto end = timer::now();
    std::cout << "Lookup: " << duration_cast<milliseconds>(end - start).count()
              << " ms." << std::endl;
  }

  // static lookup
  {
    const auto start = timer::now();
    for (size_t i = 0; i < n_iterations; ++i) {
      lookup_static(rand_int(engine));
    }
    const auto end = timer::now();
    std::cout << "Lookup (static): "
              << duration_cast<milliseconds>(end - start).count() << " ms."
              << std::endl;
  }

  return 0;
}
