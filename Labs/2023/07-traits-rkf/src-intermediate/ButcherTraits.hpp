#ifndef BUTCHER_TRAITS_HPP
#define BUTCHER_TRAITS_HPP

#include "ButcherRKF.hpp"

// utilities for binding the butcher table
// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
// https://stackoverflow.com/questions/74883308/check-if-type-is-specialization-of-template-for-both-type-and-non-type-template
// default case, it is not a template specialization
template <template <unsigned...> typename C, typename T>
struct is_template_specialization_of_unsigned_template
    : public std::false_type {};
// special case, it is a template specialization
template <template <unsigned...> typename C, unsigned... T>
struct is_template_specialization_of_unsigned_template<C, C<T...>>
    : public std::true_type {};
// concept wrapper
template <template <unsigned...> typename C, typename T>
concept IsTemplateSpecializationOfUnsignedTemplate =
    is_template_specialization_of_unsigned_template<C, std::decay_t<T>>::value;

// https://stackoverflow.com/questions/8709340/can-the-type-of-a-base-class-be-obtained-from-a-template-type-automatically
// utility to get the (base) type of a class U given the pointer to one of its
// members T::* if the member is non-virtual it refers to the base class
// unimplemented to make sure it's only used
// in unevaluated contexts (sizeof, decltype, alignof)
template <class T, class U>
T base_of(U T::*);

// we require two things for a type T to be a valid ButcherArray:
// - the base class of T is ButcherArray<N>
// - T is not an instance ButcherArray<N>
// decltype(base_of(&T::A)) is a dirty trick to get the base class of T
// however it may be T itself
template <typename T>
concept IsButcherArray =
    IsTemplateSpecializationOfUnsignedTemplate<ButcherArray,
                                               decltype(base_of(&T::A))> &&
    (!std::is_same<T, decltype(base_of(&T::A))>::value);

#endif /* BUTCHER_TRAITS_HPP */
