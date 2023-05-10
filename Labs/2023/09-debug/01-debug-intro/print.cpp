#include <iostream>

int main(int argc, char **argv) {

// Compile with `-DNDEBUG` to disable printing debug output
#ifndef NDEBUG
  std::cout << "__FILE__: " << __FILE__ << std::endl;
  std::cout << "__LINE__: " << __LINE__ << std::endl;
  std::cout << "__func__: " << __func__ << std::endl;
#endif

  return 0;
}
