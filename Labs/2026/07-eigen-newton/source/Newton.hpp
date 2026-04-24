#ifndef NEWTON_HPP
#define NEWTON_HPP

#include "Jacobian.hpp"
#include "NewtonMethodsSupport.hpp"

#include <memory>
#include <utility>

/// This class implements Newton's method.
///
/// The non-linear system to be solved is composed to this class as
/// a private member, whereas the Jacobian is stored as a unique_ptr
/// to the base class. However, as a result the class is neither
/// copy-constructible nor copy-assignable. To have copy operators that perform
/// deep copy a clone() method should be implemented in the Jacobian classes.
class Newton
{
public:
  /// Short-hand alias.
  using T = NewtonTraits;

  /// Default constructor.
  Newton() = default;

  /// This class is not meant to be copy constructable.
  /// The implementation of copy constructor would require a
  /// mechanism for the deep copy of the Jacobian, which is stored
  /// as a unique pointer.
  Newton(const Newton &) = delete;

  /// This constructor accepts a non linear system and a unique
  /// pointer to the Jacobian base class.
  // Define the proper concept (or use Newton(T::NonLinearSystemType&& system_, ...)).
  Newton(NonLinearSystemCallable auto system_,
         std::unique_ptr<JacobianBase> jac_,
         const NewtonOptions &         options_ = NewtonOptions())
    : system(std::move(system_))
    , jac(std::move(jac_))
    , options(options_)
  {}

  virtual ~Newton() = default;

  /// You can set options.
  void
  set_options(const NewtonOptions &options_)
  {
    options = options_;
  }

  [[nodiscard]] NewtonResult
  solve(const T::VariableType &x0);

private:
  T::NonLinearSystemType        system;
  std::unique_ptr<JacobianBase> jac;
  NewtonOptions                 options;
  NewtonResult                  result;
};


#endif /* NEWTON_HPP_ */
