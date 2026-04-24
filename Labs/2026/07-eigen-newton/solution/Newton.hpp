#ifndef NEWTON_HPP
#define NEWTON_HPP

#include "Jacobian.hpp"
#include "NewtonMethodsSupport.hpp"

#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

/// This class implements Newton's method.
///
/// The non-linear system to be solved is composed to this class as
/// a private member, whereas the Jacobian is stored as a unique_ptr
/// to the base class. However, as a result the class is neither
/// copy-constructible nor copy-assignable. To have copy operators that perform
/// deep copy a clone() method should be implemented in the Jacobian classes.
template <ProblemType Type>
class Newton
{
public:
  /// Short-hand alias.
  using T = NewtonTraits<Type>;

  /// Default constructor.
  Newton() = default;

  /// This class is not meant to be copy constructable.
  /// The implementation of copy constructor would require a
  /// mechanism for the deep copy of the Jacobian, which is stored
  /// as a unique pointer.
  Newton(const Newton &) = delete;

  /// This constructor accepts a non linear system and a unique
  /// pointer to the Jacobian base class.
  template <class System>
    requires NonLinearSystemCallableFor<Type, System>
  Newton(System &&                        system_,
         std::unique_ptr<JacobianBase<Type>> jac_,
         const NewtonOptions &             options_ = NewtonOptions())
    : system(std::forward<System>(system_))
    , jac(std::move(jac_))
    , options(options_)
  {}

  /// Convenience constructor taking a concrete Jacobian object.
  template <class System, class Jac>
    requires NonLinearSystemCallableFor<Type, System> &&
             JacobianImplementation<Type, std::remove_cvref_t<Jac>>
  Newton(System &&              system_,
         Jac &&                 jac_,
         const NewtonOptions &  options_ = NewtonOptions())
    : system(std::forward<System>(system_))
    , jac(std::make_unique<std::remove_cvref_t<Jac>>(std::forward<Jac>(jac_)))
    , options(options_)
  {}

  virtual ~Newton() = default;

  /// You can set options.
  void
  set_options(const NewtonOptions &options_)
  {
    options = options_;
  }

  [[nodiscard]] NewtonResult<Type>
  solve(const typename T::VariableType &x0);

private:
  typename T::NonLinearSystemType     system;
  std::unique_ptr<JacobianBase<Type>> jac;
  NewtonOptions                       options;
  NewtonResult<Type>                  result;
};

template <ProblemType Type>
NewtonResult<Type>
Newton<Type>::solve(const typename T::VariableType &x0)
{
  const auto &[tol_res, tol_incr, max_iter, stop_on_stagnation] = this->options;

  auto &[solution, norm_res, norm_incr, iteration, converged, stagnation] =
    this->result;

  solution  = x0;
  iteration = 0;
  norm_res  = std::numeric_limits<double>::max();
  norm_incr = std::numeric_limits<double>::max();

  auto residual = this->system(solution);

  if (T::size(solution) != T::size(residual))
    throw std::runtime_error("Newton needs a function from Rn to Rn");

  norm_res = T::norm(residual);
  converged  = false;
  stagnation = false;

  bool stop            = true;
  bool no_decrease_old = true;

  for (iteration = 0; iteration < max_iter; ++iteration)
    {
      const auto norm_res_old = norm_res;

      auto delta = jac->solve(solution, residual);

      norm_incr = T::norm(delta);
      solution -= delta;

      residual = this->system(solution);
      norm_res = T::norm(residual);

      const bool no_decrease = (norm_res >= norm_res_old);
      stagnation = (no_decrease_old && no_decrease);
      stop = (stagnation && stop_on_stagnation);
      converged = ((norm_res <= tol_res) && (norm_incr <= tol_incr));

      std::cout << "    Iteration " << iteration;
      std::cout << ", residual: " << norm_res;
      std::cout << ", increment: " << norm_incr;
      std::cout << std::endl;

      if (converged || stop)
        break;

      no_decrease_old = no_decrease;
    }

  return result;
}

#endif /* NEWTON_HPP_ */
