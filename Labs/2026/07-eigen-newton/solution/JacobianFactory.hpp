#ifndef JACOBIANFACTORY_HPP
#define JACOBIANFACTORY_HPP

#include "Jacobian.hpp"

#include <memory>
#include <utility>

enum class JacobianType : unsigned int
{
  Discrete = 0,
  Full     = 1
};

template <ProblemType Type, JacobianType JacType, class... Args>
[[nodiscard]] std::unique_ptr<JacobianBase<Type>>
make_jacobian(Args &&... args)
{
  static_assert(JacType == JacobianType::Discrete ||
                  JacType == JacobianType::Full,
                "Error in JacobianType: wrong type specified.");

  if constexpr (JacType == JacobianType::Discrete)
    {
      static_assert(std::constructible_from<DiscreteJacobian<Type>, Args...>);
      return std::make_unique<DiscreteJacobian<Type>>(
        std::forward<Args>(args)...);
    }
  else
    {
      static_assert(std::constructible_from<FullJacobian<Type>, Args...>);
      return std::make_unique<FullJacobian<Type>>(std::forward<Args>(args)...);
    }
}

#endif /* JACOBIANFACTORY_HPP */
