#include "Derivatives.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <chrono>

auto timeit(const std::function<void()> &f, size_t N = 1) {
  using namespace std::chrono;
  const auto t0 = high_resolution_clock::now();
  for(size_t i = 0; i < N; ++i)
    f();
  const auto t1 = high_resolution_clock::now();
  return duration_cast<milliseconds>(t1 - t0).count();
}


int main(int argc, char **argv) {
  const size_t N = 1e8; 
  const double h = 1e-2;

  const auto f = [](double x) { return x*x*x*x*x; };
  const auto df4 = [](double x) { return 5.0*4.0*3.0*2.0*x; };

  const auto test_manual = [&]() {
    const auto y = df4(1.0);
  };

  const auto test_naive_v1 = [&]() {
    const auto y = NaiveNthDerivative{f, h, 4}(1.0);
  };

  const auto test_naive_v2 = [&]() {
    const auto y = NaiveAlternatingNthDerivative{f, h, 4}(1.0);
  };

  const auto test_template = [&]() {
    const auto y = make_forward_derivative<4>(f, h)(1.0);
  };

  const auto test_template_v1 = [&]() {
    const auto y = TemplateRecursionNthDerivative<4>{f, h}(1.0);
  };

  const auto test_template_v2 = [&]() {
    const auto y = TemplateBoolRecursionNthDerivative<4, true>{f, h}(1.0);
  };

  const auto test_template_v3 = [&]() {
    const auto y (NthDerivative<4, std::function<double(double)>, 
                  double, Differentiation::FORWARD >{f, h}(1.0));
  };

  std::cout << "Test hard coded:               " << timeit(test_manual, N) << "[ms]\n";
  std::cout << "Test naive impl:               " << timeit(test_naive_v1, N) << "[ms]\n";
  std::cout << "Test naive impl (alternating): " << timeit(test_naive_v2, N) << "[ms]\n";
  std::cout << "Test templated (v1):           " << timeit(test_template_v1, N) << "[ms]\n";
  std::cout << "Test templated (v2):           " << timeit(test_template_v2, N) << "[ms]\n";
  std::cout << "Test templated (v3):           " << timeit(test_template_v3, N) << "[ms]\n";
  std::cout << "Test templated (full):         " << timeit(test_template, N) << "[ms]\n";
  std::cout << "Test hard coded:               " << timeit(test_manual, N) << "[ms]\n";
  std::cout << "check accuracy: " << NthDerivative<4, std::function<double(double)>>{f, h}(1.0) - df4(1.0) << std::endl;
  std::cout << "check accuracy: " << make_forward_derivative<4>(f, h)(1.0) - df4(1.0) << std::endl;
}

