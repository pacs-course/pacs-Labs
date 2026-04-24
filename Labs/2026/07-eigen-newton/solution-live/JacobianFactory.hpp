#ifndef JACOBIANFACTORY_HPP
#define JACOBIANFACTORY_HPP

#include "Jacobian.hpp"

#include <memory>
#include <utility>

enum class JacobianType : unsigned int
{
  Discrete = 0,
  Full = 1
};

// factory returning a JBase (polym...) via smart pointer
// shared_ptr <-> const type &
// unique_ptr <-> &&, std::move

template <JacobianType JacType, class... Args> //varyadic templates
std::unique_ptr <JacobianBase> 
make_jacobian 
(Args &&... args)
{
  static_assert (JacType == JacobianType::Discrete || JacType == JacobianType::Full, "Error....wrong type"); 

  if constexpr (JacType == JacobianType::Discrete)
    {
      static_assert (std::constructible_from <DiscreteJacobian,Args...>);
      return std::make_unique <DiscreteJacobian> (std::forward<Args>(args)...); //perfect forwarding
    }
  else
    {
      static_assert (std::constructible_from <FullJacobian,Args...>);
      return std::make_unique <FullJacobian> (std::forward<Args>(args)...);
    }
}

#endif /* JACOBIANFACTORY_HPP */
