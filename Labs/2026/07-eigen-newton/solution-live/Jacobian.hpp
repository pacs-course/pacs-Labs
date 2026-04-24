#ifndef JACOBIAN_HPP
#define JACOBIAN_HPP

#include "NewtonTraits.hpp"

/// The base class for representing the Jacobian.
///
/// It implements the basic methods for the application of the
/// Jacobian, which in fact may be a "pseudo-Jacobian" for
/// quasi-Newton methods or even Picard iterations.
class JacobianBase
{
public:
  /// Short-hand alias.
  using T = NewtonTraits;

  /// Default constructor.
  JacobianBase() = default;

  /// Solve J(x) * delta_x = res.
  [[nodiscard]] virtual T::VariableType
  solve(const T::VariableType &x, const T::VariableType &res) const = 0;

  /// Destructor. Needed since this is a pure virtual class.
  virtual ~JacobianBase() = default;
};

class FullJacobian final : public JacobianBase
{
  public:
    explicit FullJacobian (JacobianCallable auto jac_) : jac(jac_) {}

    [[nodiscard]] T::VariableType 
    solve (const T::VariableType &x, const T::VariableType &res) const override;
  
  private:
    T::JacobianFunctionType jac;

};

class DiscreteJacobian final : public JacobianBase
{
  public:  
    explicit DiscreteJacobian (NonLinearSystemCallable auto system_, double h_)
    : system(system_), h(h_) {}
  
    [[nodiscard]] T::VariableType 
    solve (const T::VariableType &x, const T::VariableType &res) const override;
  private:
    T::NonLinearSystemType system;
    double h;
};

#endif /* JACOBIAN_HPP */
