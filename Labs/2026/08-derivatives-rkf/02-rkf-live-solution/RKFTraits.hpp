#ifndef RKFTRAITS_HPP
#define RKFTRAITS_HPP

#include <Eigen/Dense>

#include <cmath>
#include <functional>
#include <vector>

enum class RKFKind
{
  SCALAR = 0,
  VECTOR = 1,
  MATRIX = 2
};

template <RKFKind Kind>
struct RKFTraits
{};


template <>
struct RKFTraits < RKFKind::SCALAR>
{
  using VariableType = double;
  using ForcingTermType = std::function <VariableType (const double &, const VariableType &)>;
  static double norm (const VariableType &x) {return std::abs(x);} 
};


template <>
struct RKFTraits < RKFKind::VECTOR>
{
  using VariableType = Eigen::VectorXd;
  using ForcingTermType = std::function <VariableType (const double &, const VariableType &)>;
  static double norm (const VariableType &x) {return x.norm();} 
};

template <>
struct RKFTraits < RKFKind::MATRIX>
{
  using VariableType = Eigen::MatrixXd;
  using ForcingTermType = std::function <VariableType (const double &, const VariableType &)>;
  static double norm (const VariableType &x) {return x.norm();} 
};


#endif /* RKFTRAITS_HPP */
