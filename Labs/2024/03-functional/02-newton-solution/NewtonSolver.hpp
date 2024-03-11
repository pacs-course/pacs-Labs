#ifndef NEWTONSOLVER_H
#define NEWTONSOLVER_H

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>

class NewtonSolver
{
public:
  NewtonSolver(const std::function<double(const double &)> &fun_,
               const std::function<double(const double &)> &dfun_,
               const unsigned int                           n_max_it_ = 100,
               const double tol_fun_ = 
                    std::numeric_limits<double>::epsilon() * 1000.0,
               const double tol_x_   = 
                    std::numeric_limits<double>::epsilon() * 1000.0)
    : m_fun(fun_)
    , m_dfun(dfun_)
    , m_n_max_it(n_max_it_)
    , m_tol_fun(tol_fun_)
    , m_tol_x(tol_x_)
    , m_x(0)
    , m_df_dx(0)
    , m_dx(0)
    , m_res(0)
    , m_iter(0)
  {}

  void solve(const double &x0)
  {
    // initial guess
    m_x = x0;

    for (m_iter = 0; m_iter < m_n_max_it; ++m_iter)
      {
        // compute function residual
        m_res = m_fun(m_x);

        // check convergence on the residual
        if (std::abs(m_res) < m_tol_fun)
          break;

        // evaluate function derivative at current point
        m_df_dx = m_dfun(m_x);

        // Newton update
        m_dx = m_res / m_df_dx;
        m_x -= m_dx;

        // check convergence on the increment
        if (std::abs(m_dx) < m_tol_x)
          break;
      }
  }

  double  get_result() const
  {
    return m_x;
  };

  double get_residual() const
  {
    return m_res;
  };

  unsigned int get_iter() const
  {
    return m_iter;
  };

private:

  // Variables to initialize the Newton method
  // function f
  std::function<double(const double &)> m_fun;

  // derivative of f
  std::function<double(const double &)> m_dfun;

  // number of maximum iteration
  const unsigned int m_n_max_it;

  // tolerance on the residual
  const double       m_tol_fun;

  // tolerance on the increment
  const double       m_tol_x;

  // Variables employed by the solver
  // guess on the solution
  double       m_x;

  // value of the derivative at point x
  double       m_df_dx;

  // increment 
  double       m_dx;

  // residual
  double       m_res;

  // number of iterations
  unsigned int m_iter;
};

#endif /* NEWTONSOLVER_H */
