#ifndef NEWTONTRAITS_HPP
#define NEWTONTRAITS_HPP

#include <cmath>
#include <concepts>
#include <functional>

#include <Eigen/Dense>

enum class ProblemType : unsigned int
{
  Scalar = 0,
  Vector = 1
};

template <ProblemType Type>
class NewtonTraits
{};

template <>
class NewtonTraits<ProblemType::Scalar>
{
public:
  using VariableType = double;
  using JacobianMatrixType = double;
  using NonLinearSystemType = std::function<VariableType(const VariableType &)>;
  using JacobianFunctionType =
    std::function<JacobianMatrixType(const VariableType &)>;

  static constexpr size_t
  size(const VariableType & /*x*/)
  {
    return 1;
  }

  static double
  norm(const VariableType &x)
  {
    return std::abs(x);
  }
};

template <>
class NewtonTraits<ProblemType::Vector>
{
public:
  using VariableType = Eigen::VectorXd;
  using JacobianMatrixType = Eigen::MatrixXd;
  using NonLinearSystemType = std::function<VariableType(const VariableType &)>;
  using JacobianFunctionType =
    std::function<JacobianMatrixType(const VariableType &)>;

  static size_t
  size(const VariableType &x)
  {
    return x.size();
  }

  static double
  norm(const VariableType &x)
  {
    return x.norm();
  }
};

template <ProblemType Type, class Callable>
concept NonLinearSystemCallableFor =
  std::invocable<Callable, const typename NewtonTraits<Type>::VariableType &> &&
  std::same_as<std::invoke_result_t<
                 Callable, const typename NewtonTraits<Type>::VariableType &>,
               typename NewtonTraits<Type>::VariableType>;

template <ProblemType Type, class Callable>
concept JacobianCallableFor =
  std::invocable<Callable, const typename NewtonTraits<Type>::VariableType &> &&
  std::same_as<std::invoke_result_t<
                 Callable, const typename NewtonTraits<Type>::VariableType &>,
               typename NewtonTraits<Type>::JacobianMatrixType>;

#endif /* NEWTONTRAITS_HPP */
