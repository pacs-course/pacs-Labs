#include "Jacobian.hpp"

FullJacobian::T::VariableType 
FullJacobian::solve (const T::VariableType &x, const T::VariableType &res) const
{
  const T::JacobianMatrixType J = jac (x);
  return J.fullPivLu().solve(res);
}

DiscreteJacobian::T::VariableType 
DiscreteJacobian::solve (const T::VariableType &x, const T::VariableType &res) const
{
  const size_t n = x.size ();

  T::JacobianMatrixType J (n,n);
  
  for (size_t i = 0; i < n; ++i)
    {
      T::VariableType x_m (x);
      T::VariableType x_p (x);

      x_m[i] -= h;
      x_p[i] += h;

      const T::VariableType f_m = system (x_m);
      const T::VariableType f_p = system (x_p);

      J.col (i) = (f_p - f_m)/(2.*h);
    }

  return J.fullPivLu().solve(res);
}

