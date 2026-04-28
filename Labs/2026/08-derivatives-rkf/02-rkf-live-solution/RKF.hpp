#ifndef RKF_HPP
#define RKF_HPP

#include "ButcherRKF.hpp"
#include "RKFTraits.hpp"

#include <cmath>
#include <functional>
#include <vector>
#include <iostream>
#include <limits>
#include <utility>


/*
RKFResult

RKF class

methods
   - step
  - operator ()
*/

template <RKFKind KIND>
struct RKFResult
{
  std::vector <double> time;
  std::vector <typename RKFTraits <KIND> :: VariableType> y;
  double estimatedError {0.0};
  bool failed{false};
  int expansions{0};
  int contractions {0};
};

template <class B, RKFKind KIND = RKFKind::SCALAR> //method, dimension
class RKF : public RKFTraits <KIND>
{
  public:
    using VariableType = typename RKFTraits<KIND>::VariableType;
    using Function = typename RKFTraits<KIND>::ForcingTermType;

    //a constructor
    RKF (const B& bt, const Function &f) : ButcherTable (bt), M_f (f) {};

    //def. constructor
    RKF () = default;
    
    //setters
    void set_funtion (const Function & f) {M_f = f;}
    void set_ButcherArray (const B & bt) {ButcherTable = bt;}

    /*!
     * @param t0 initial time
     * double y0           = 1;
     * double T            = 100;
     * double h_init       = 0.2;
     * double errorDesired = 1.e-4; 
     * maxSteps (in the class) a maxSteps (default)
     */
    RKFResult <KIND> operator () (const double t0, const double T, 
                                  const VariableType & y0,
                                  const double hInit, const double tol,
                                  int maxSteps = 2000) const;
  private:
    B ButcherTable;
    Function M_f;

    // private single step going to be called by public operator ()
    std::pair <VariableType, VariableType> RKFStep (const double tstart,
                                                    const VariableType &y0,
                                                    const double h) const;

};

// Implementations.
inline std::ostream &
operator<<(std::ostream &out, const RKFResult<RKFKind::SCALAR> &res);

inline std::ostream &
operator<<(std::ostream &out, const RKFResult<RKFKind::VECTOR> &res); 

std::ostream &
operator<<(std::ostream &out, const RKFResult<RKFKind::SCALAR> &res)
{
  out << "# Number ot time steps:" << res.time.size()
      << " N. contractions:" << res.contractions
      << " N. expansions:" << res.expansions << std::endl;
  out << "#   t    y   Estimated error=" << res.estimatedError
      << std::endl;
  double hmin = res.time[1] - res.time[0];
  double hmax = hmin;
  for (unsigned int i = 0; i < res.time.size() - 1; ++i)
    {
      auto delta = res.time[i + 1] - res.time[i];
      hmax       = std::max(hmax, delta);
      hmin       = std::min(hmin, delta);
    }
  out << "# hmin:" << hmin << " hmax:" << hmax << std::endl;
  std::size_t i = 0;
  for (auto const &t : res.time)
    out << t << " " << res.y[i++] << "\n";
  return out;
}

template <class B, RKFKind KIND>
RKFResult <KIND> 
RKF<B,KIND>::operator () (const double T0, const double T, 
                          const VariableType & y0,
                          const double hInit, const double tol,
                          int maxSteps) const
{
  RKFResult <KIND> res;

  //aliases and initializations
  std::vector <double>            & time              = res.time;
  std::vector <VariableType>      & y                 = res.y;
  auto                            & estimatedError    = res.estimatedError;
  auto                            & failed            = res.failed;
  int                             & expansions        = res.expansions;
  int                             & contractions      = res.contractions;

  //vectors: reserve
  int expectedSteps = std::min (maxSteps, std::max (1, 1 + static_cast <int> ((T-T0)/hInit)));
  time.reserve (expectedSteps);
  y.reserve (expectedSteps);
  time.push_back (T0);
  y.push_back (y0);

  bool rejected (false);
  double constexpr reductionFactor = .95;
  double constexpr expansionFactor = 2.;

  double factor;
  if (ButcherTable.order <= 2)
    factor = 1.;
  else
    factor = 1./ (ButcherTable.order -1);

  //init loop over T/iters
  int iter = 0.;
  double t = T0;
  double delta = T - T0;
  double hmin = 100*(T-T0)*std::numeric_limits <double>::epsilon ();
  double h = std::max (hInit, hmin);
  VariableType ycurr = y0;

  while (t < T && iter <= maxSteps)
  {
    ++iter;
    VariableType ylow, yhigh;

    double errorPerTimeStep = tol * h / delta;

    if (t+h >= T)
    {
      h = T - t;
      if (h < hmin) //very small
        {
          ylow = ycurr;
          yhigh = ycurr;
        }
      else
        std::tie (ylow, yhigh) = RKFStep (t, ycurr, h); //do one last step
    }
    else
      std::tie (ylow, yhigh) = RKFStep (t, ycurr, h); //step
    
    double currentError = this -> norm (ylow - yhigh);
    double mu = std::pow (errorPerTimeStep / currentError, factor);

    if (currentError <= errorPerTimeStep)
    {
      t = t +h;
      time.push_back (t);
      y.push_back (yhigh); 
      ycurr = yhigh;
      estimatedError += currentError;
      if ((mu>= 1.) && !rejected && (t<T))
        {
          h *= std::min (expansionFactor, mu);
          ++expansions;
        }
      rejected = false;
    }//if
    else
    {
      rejected = true;
      h *= mu * reductionFactor;
      ++contractions;
      h = h <= hmin ? hmin : h;
    }
  }//while
  
  if (iter > maxSteps)
    failed = true;

  return res;
}


template <class B, RKFKind KIND>
auto  RKF<B,KIND>:: RKFStep 
(const double tstart, const VariableType &y0, const double h) const
    -> std::pair <VariableType, VariableType>
{
  /*
    do a single RKF step with a fixed h:
     1. compute K[i] (i = 0:Nstages-1)
     2. get high and low order solution
  */

  auto constexpr Nstages = B::Nstages ();
  std::array <VariableType, Nstages> K;

  //define aliases via references
  typename B::Atable const           &          A = ButcherTable.A;
  const std::array <double, Nstages> &          c  {ButcherTable.c};
  const std::array <double, Nstages> &          b1 {ButcherTable.b1};
  const std::array <double, Nstages> &          b2 {ButcherTable.b2};


  //1. Compute K
  //first step (euler)
  K[0] = M_f (tstart, y0) * h;
  //\(k_i = f\left(t_n + c_i h_n, y_n + \sum_{j=1}^{i-1}a_{ij}k_j\right)\), and \(h_n = t_{n+1} - t_n\).
  for (unsigned int i = 1; i < Nstages; ++i)
    {
      double time = tstart + c[i] * h;
      VariableType v = y0;
      for (unsigned int j = 0; j < i; ++j)
        v += A[i][j] * K[j];
      K[i] = M_f (time, v) * h;
    }
  
  // 2. Compute high (v1) and low (v2) order solution
  VariableType v1 = y0;
  VariableType v2 = y0;

  for (unsigned int i = 0; i < Nstages; ++i)
    {
      v1 += K[i] * b1[i];
      v2 += K[i] * b2[i];
    }

  return std::make_pair (v1,v2);
}





















#endif /* RKF_HPP */
