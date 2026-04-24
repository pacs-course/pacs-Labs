#ifndef NEWTONTRAITS_HPP
#define NEWTONTRAITS_HPP

#include <Eigen/Dense>
#include <functional>

#include <concepts>

class NewtonTraits
{
  public:
    using VariableType = Eigen::VectorXd;
    using JacobianMatrixType = Eigen::MatrixXd;

    using NonLinearSystemType = std::function <VariableType (const VariableType &)>;
    using JacobianFunctionType = std::function <JacobianMatrixType (const VariableType &)>;
};

template <class Callable> 
concept NonLinearSystemCallable = 
  std::invocable <Callable, const NewtonTraits::VariableType &> && 
  std::same_as <std::invoke_result_t <Callable, const NewtonTraits::VariableType &>,
                NewtonTraits::VariableType>;

template <class Callable>
concept JacobianCallable = 
  std::invocable <Callable, const NewtonTraits::VariableType> &&
  std::same_as <std::invoke_result_t <Callable, const NewtonTraits::VariableType &>,
                NewtonTraits::JacobianMatrixType>;

#endif /* NEWTONTRAITS_HPP */
