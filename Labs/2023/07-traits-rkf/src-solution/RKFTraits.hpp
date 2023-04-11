#ifndef RKFTRAITS_HPP
#define RKFTRAITS_HPP

#include <cmath>
#include <functional>
#include <type_traits>
#include <concepts>

// Define a scalar
template<typename T>
concept Scalar = std::is_floating_point<T>::value;

// Define a vector
// by the definition of (normed) vectorial space 
template<typename T>
concept Normable = requires(T x) {
  { x.norm() } -> Scalar;
};

template<typename T>
concept Addable = requires(T a, T b) {
  { a += b } -> std::convertible_to<T>;
  { a - b }  -> std::convertible_to<T>;
};

template<typename T>
concept ScalarMultiplicable = requires(T x) {
  { 1.0  * x } -> std::convertible_to<T>;
  { 1.0f * x } -> std::convertible_to<T>;
};

template<typename T>
concept Vector = Normable<T> && Addable<T> && ScalarMultiplicable<T>;

// We want to use our ODE solver for both scalar and vectorial problems
template<typename T>
concept ScalarOrVector = Scalar<T> || Vector<T>;

#endif /* RKFTRAITS_HPP */
