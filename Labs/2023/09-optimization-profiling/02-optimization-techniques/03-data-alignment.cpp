#include <iostream>

struct Struct0 {
   uint64_t u64; // 8 bytes
   uint32_t u32; // 4 bytes
   // Padding here
};

class Struct1 {
  char c;       // 1 byte.
  int i;        // 4 bytes.
  short int s;  // 2 bytes.
};

int main() {
  std::cout << "Size of Struct0 is " << sizeof(Struct0) << " bytes." << std::endl;
  std::cout << "Size of Struct1 is " << sizeof(Struct1) << " bytes." << std::endl;

  return 0;
}
