#include <iostream>
#include <limits>

int
main(int argc, char **argv)
{
  // when we use -O3 optimization the compiler does something like this
  // (1) because every iteration with i of any value larger than 1 has undefined behavior 
  // (2) we can assume that i <= 1 for optimization purposes 
  // (3) the loop condition is always true 
  // (4) it's optimized away into an infinite loop
  // we can "confirm" that this is the reasoning of the compiler by adding
  // `if(i == 1) std::exit(1);` or `if(i == 2) std::exit(1);`
  // in the first case we exit since the compiler is supposing i <= 1, so sees the branch
  // in the second case we remain in the loop since i > 2 and the compiler thinks i 
  // cannot be 2 and the branch is discarded
  // see also: https://stackoverflow.com/a/24296605
  for (int i = 0; i < 10; ++i)
    {
      // The correct way to write this expression is to use brackets
      // to ensure proper operation priority
      // const double y = i * (std::numeric_limits<int>::max() / 1.0e10);
      const double y = i * std::numeric_limits<int>::max() / 1.0e10;

      std::cout << y << std::endl;
    }

  return 0;
}
