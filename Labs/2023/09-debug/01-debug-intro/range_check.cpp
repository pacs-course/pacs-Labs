// add the flag: `-fsanitize=address`
// WARNING: not compatible with Valgrind 
// see also: https://stackoverflow.com/questions/42079091/valgrind-gcc-6-2-0-and-fsanitize-address
#include <vector>
#include <iostream>

int main() {
    std::vector<uint64_t> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << v[11] << "\n";
    for(const auto e : v)
        std::cout << e;
    return 0;
}