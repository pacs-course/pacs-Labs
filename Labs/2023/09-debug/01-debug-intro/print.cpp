#include <chrono>
#include <iostream>

// Compile with `-DNDEBUG` to disable printing debug output
#ifdef NDEBUG
#define DEBUG_STDOUT(x) \
  do {                  \
  } while (0)
#else
#define DEBUG_STDOUT(x)                                                  \
  do {                                                                   \
    using namespace std::chrono;                                         \
    std::cout << duration_cast<milliseconds>(                            \
                     system_clock::now().time_since_epoch())             \
                     .count()                                            \
              << " | " << __func__ << "@" << __LINE__ << "(" << __FILE__ \
              << ") | " << x << std::endl;                               \
  } while (0)
#endif

int main(int argc, char **argv) {
  std::cout << "__FILE__: " << __FILE__ << std::endl;
  std::cout << "__LINE__: " << __LINE__ << std::endl;
  std::cout << "__func__: " << __func__ << std::endl;

  DEBUG_STDOUT("test message");

  return 0;
}
