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
  { a + b } -> std::convertible_to<T>;
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

// utilities for binding the butcher table
// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
// https://stackoverflow.com/questions/74883308/check-if-type-is-specialization-of-template-for-both-type-and-non-type-template
template <template <unsigned...> typename C, typename T>
struct non_type_instance_impl : public std::false_type {};
template <template <unsigned...> typename C, unsigned... T>
struct non_type_instance_impl<C, C<T...>> : public std::true_type {};
template <template <unsigned...> typename C, typename T>
concept instance_of_non_type =
    non_type_instance_impl<C, std::decay_t<T>>::value;

// https://stackoverflow.com/questions/8709340/can-the-type-of-a-base-class-be-obtained-from-a-template-type-automatically
// unimplemented to make sure it's only used
// in unevaluated contexts (sizeof, decltype, alignof)
template <class T, class U>
T base_of(U T::*);

#endif /* RKFTRAITS_HPP */
