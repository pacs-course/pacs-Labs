#pragma once

#include <functional>

/*
TODO
  1. general template d. NthDerivative<...>
  2. draft recursion -> implement -> test
  3. implement stopping criterion by template specialisation
  4. Alternate DifferenceType::FORWARD. BACKWARD
*/

namespace DifferenceType
{
  struct BACKWARD; //forward declarations
  
  struct FORWARD
  {
    using OtherType = BACKWARD;
  };

  struct BACKWARD
  {
    using OtherType = FORWARD;
  };
}

template <unsigned N,
          typename F,
          typename T = double,
          typename DT = DifferenceType::FORWARD>
class NthDerivative
{
  public:
    using PreviousDerivative = NthDerivative <N-1, F, T, 
    typename DT::OtherType>; // 2.73194 
    //DT>;  2.7594
    
    NthDerivative (const F & f, const T & h_): pDerivative {f, h_}, h(h_) {}; //not pDerivative(f) only! try to see

    T operator() (const T &x) const
    {
      if constexpr ( std::is_same_v <DifferenceType::FORWARD, DT> )
        return (pDerivative(x+h) - pDerivative(x))/h;
      else
        return (pDerivative(x) - pDerivative(x-h))/h;
    }
  
  private:
    PreviousDerivative pDerivative;
    T h;
};

//specialisation -- stopping criterion for template recursion

template <//unsigned N,
          typename F,
          typename T,
          typename DT>
class NthDerivative <1u, F, T, DT>
{
  public:
    NthDerivative (const F & f, const T & h_): pDerivative {f}, h(h_) {};

    T operator() (const T &x) const
    {
      if constexpr ( std::is_same_v <DifferenceType::FORWARD, DT> )
        return (pDerivative(x+h) - pDerivative(x))/h;
      else
        return (pDerivative(x) - pDerivative(x-h))/h;
    }
  
  private:
    F pDerivative;
    T h;
};





















