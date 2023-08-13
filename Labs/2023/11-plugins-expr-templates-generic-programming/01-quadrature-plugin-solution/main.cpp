#include <dlfcn.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <numbers>

double integrand(double x) { return std::sin(x * x) * std::sin(x * x); }

int main(int argc, char **argv) {
  // About RTLD_LAZY vs RTLD_NOW
  // https://pubs.opengroup.org/onlinepubs/009696799/functions/dlopen.html
  // https://stackoverflow.com/a/63461361
  void *handle = dlopen(argv[1], RTLD_LAZY);
  if (!handle) {
    std::cerr << "Cannot load object!" << std::endl;
    std::cerr << dlerror() << std::endl;

    return 1;
  }

  void *sym = dlsym(handle, "integrate");
  if (!sym) {
    std::cerr << "Cannot load symbol!" << std::endl;
    std::cerr << dlerror() << std::endl;

    return 1;
  }

  double (*integrate)(const std::function<double(double)> &, double, double);
  integrate = reinterpret_cast<decltype(integrate)>(sym);

  const double res = integrate(integrand, 0, std::numbers::pi_v<double>);

  std::cout << "res = " << res << std::endl;

  return 0;
}
