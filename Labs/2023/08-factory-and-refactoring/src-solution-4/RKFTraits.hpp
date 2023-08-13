#ifndef RKFTRAITS_HPP
#define RKFTRAITS_HPP

#include <type_traits>

// Define a type that is convertible to a scalar (e.g. double& is accepted)
template <typename T>
concept Scalar = std::is_floating_point<std::decay_t<T>>::value;

// Define a vector
// by the definition of (normed) vectorial space
template <typename T>
concept Normable = requires(T x) {
                     { x.norm() } -> Scalar;
                   };

template <typename T>
concept Addable = requires(T a, T b) {
                    { a += b } -> std::convertible_to<T>;
                    { a - b } -> std::convertible_to<T>;
                  };

template <typename T>
concept ScalarMultiplicable = requires(T x) {
                                { 1.0 * x } -> std::convertible_to<T>;
                                { 1.0f * x } -> std::convertible_to<T>;
                              };

// To output the solution to a file we need the object to be subscriptable
template <typename T>
concept Subscriptable = requires(T x) {
                          { x[0] } -> Scalar;
                          { x.size() } -> std::integral;
                        };

template <typename T>
concept Vector = Normable<T> && Addable<T> && ScalarMultiplicable<T> &&
                 Subscriptable<T> && std::is_default_constructible<T>::value;

// We want to use our ODE solver for both scalar and vectorial problems
template <typename T>
concept ScalarOrVector = Scalar<T> || Vector<T>;

// https://quuxplusone.github.io/blog/2022/01/04/test-constexpr-friendliness/
// https://stackoverflow.com/questions/55288555/c-check-if-statement-can-be-evaluated-constexpr
// The overload that returns true is selected when and only when F{}()
// can appear within a template argument, which effectively requires the
// lambda invocation to be a constant expression.
// We are using the comma operator in a SFINAE context
// Recall the comma operator (https://en.wikipedia.org/wiki/Comma_operator),
// represented by the token `,`, is a binary operator
// that evaluates its first operand and discards the result, and then evaluates
// the second operand and returns this value (and type); there is a sequence
// point between these evaluations.
template <class F, int = (F{}(), 0)>
constexpr bool is_constexpr_friendly(F) {
  return true;
}
constexpr bool is_constexpr_friendly(...) { return false; }

#endif /* RKFTRAITS_HPP */
