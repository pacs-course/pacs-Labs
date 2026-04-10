#include <iostream>
#include <vector>

template <typename T>
std::ostream & print_container (const T& c, std::ostream & os = std::cout)
{
    for (const auto & r : c)
        os << r << " ";
    return os;
}

int main ()
{
  std::vector <int> v {0,1,2,3,4,5,6,7,8,9};
  print_container (v) << std::endl;
  return 0;
}

