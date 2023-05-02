#include <iostream>

class Struct1 {
  char c;       // 1 byte.
  int i;        // 4 bytes.
  short int s;  // 2 bytes.
};

int main() {
  std::cout << "Size of Struct1 is " << sizeof(Struct1) << " bytes." << std::endl;

  return 0;
}
