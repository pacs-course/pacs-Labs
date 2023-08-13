#include <cassert>
#include <iostream>

// see also: https://en.cppreference.com/w/cpp/error/assert
int
main(int argc, char **argv)
{
  assert(2 + 2 == 4);
  std::cout << "Execution continues past the first assert." << std::endl;
  // you can also add a message
  assert(2 + 2 == 5 && "expected fail");
  std::cout << "Execution continues past the second assert." << std::endl;

  return 0;
}
