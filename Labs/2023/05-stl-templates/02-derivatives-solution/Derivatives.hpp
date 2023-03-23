#ifndef DERIVATIVES_HPP
#define DERIVATIVES_HPP

#include <functional>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

using func_t = std::function<double(double)>;

class NaiveNthDerivative {
public:
  NaiveNthDerivative(const func_t &f, double h, unsigned int deg) : 
    m_f(f), 
    m_h(h), 
    m_deg(deg) {}
  double operator()(double x) {
    return f_prime(x, m_f, m_deg);
  }
private:
  double f_prime(double x, func_t f, unsigned deg) {
    if(deg == 0)
      return f(x);
    if(deg == 1)
      return (f(x + m_h) - f(x)) / m_h;
    return (f_prime(x + m_h, f, deg - 1) - f_prime(x, f, deg - 1)) / m_h;
  }

  const func_t m_f;
  const double m_h;
  const unsigned int m_deg;
};

///////////////////////////////////////////////////////////////////////////////

class NaiveAlternatingNthDerivative {
public:
  NaiveAlternatingNthDerivative(const func_t &f, double h, unsigned int deg) : 
    m_f(f), 
    m_h(h), 
    m_deg(deg) {}
  double operator()(double x) {
    return f_prime(x, m_f, m_deg, true);
  }
private:
  double f_prime(double x, func_t f, unsigned deg, bool is_fw) {
    if(deg == 0)
      return f(x);
    if(is_fw)
      return (f_prime(x + m_h, f, deg - 1, !is_fw) - f_prime(x, f, deg - 1, !is_fw)) / m_h;
    else
      return (f_prime(x, f, deg - 1, !is_fw) - f_prime(x - m_h, f, deg - 1, !is_fw)) / m_h;
  }

  const func_t m_f;
  const double m_h;
  const unsigned int m_deg;
};

///////////////////////////////////////////////////////////////////////////////

template<unsigned DEG>
class TemplateRecursionNthDerivative {
public:
  TemplateRecursionNthDerivative(const func_t &f, double h) : 
    m_f(f, h), 
    m_h(h) {}
  double operator()(double x) const {
    return (m_f(x + m_h) - m_f(x)) / m_h;
  }
private:
  using PrevDerivative = TemplateRecursionNthDerivative<DEG - 1>; 
  const PrevDerivative m_f;
  const double m_h;
};

template<>
class TemplateRecursionNthDerivative<0u> {
public:
  TemplateRecursionNthDerivative(const func_t &f, double h) : 
    m_f(f), 
    m_h(h) {}
  double operator()(double x) const {
    return m_f(x);
  }
private:
  const func_t m_f;
  const double m_h;
};

///////////////////////////////////////////////////////////////////////////////

template<unsigned DEG, bool IS_FORWARD>
class TemplateBoolRecursionNthDerivative {
public:
  TemplateBoolRecursionNthDerivative(const func_t &f, double h) : 
    m_f(f, h), 
    m_h(h) {}
  double operator()(double x) const {
    if constexpr(IS_FORWARD)
      return (m_f(x + m_h) - m_f(x)) / m_h;
    else
      return (m_f(x) - m_f(x - m_h)) / m_h;
  }
private:
  using PrevDerivative = TemplateBoolRecursionNthDerivative<DEG - 1, !IS_FORWARD>; 
  const PrevDerivative m_f;
  const double m_h;
};

template<bool IS_FORWARD>
class TemplateBoolRecursionNthDerivative<0u, IS_FORWARD> {
public:
  TemplateBoolRecursionNthDerivative(const func_t &f, double h) : 
    m_f(f), 
    m_h(h) {}
  double operator()(double x) const {
    return m_f(x);
  }
private:
  const func_t m_f;
  const double m_h;
};

///////////////////////////////////////////////////////////////////////////////

namespace Differentiation
{
  class BACKWARD; // Forward declaration.

  class FORWARD
  {
  public:
    using OTHER_TYPE = BACKWARD;
  };

  class BACKWARD
  {
  public:
    using OTHER_TYPE = FORWARD;
  };
} // namespace Differentiation

/**
 * Computes the Nth derivative of a function by finite differences.
 *
 * @tparam N The order of the derivative.
 * @tparam FunctionType The callable object of signature T (T const &).
 * @tparam NumberType The argument and return type of the callable object.
 * @tparam DifferenceType The type of differencing: either
 * Differentiation::FORWARD or Differentiation::BACKWARD.
 */
template <unsigned int N,
          class FunctionType,
          class NumberType     = double,
          class DifferenceType = Differentiation::FORWARD>
class NthDerivative
{
public:
  /**
   * Constructor.
   *
   * @param f The function (callable object).
   * @param h The spacing to be used in the formula.
   */
  NthDerivative(const FunctionType &f, const NumberType &h)
    : previous_derivative{f, h}
    , h{h}
  {}

  /**
   * The call operator that computes the derivative.
   *
   * @param x The point where the derivative is computed.
   * @return The derivative value.
   */
  NumberType
  operator()(const NumberType &x) const
  {
    // Since C++17.
    if constexpr (std::is_same_v<Differentiation::FORWARD, DifferenceType>)
      return (previous_derivative(x + h) - previous_derivative(x)) / h;
    else
      return (previous_derivative(x) - previous_derivative(x - h)) / h;
  }

private:
  /// The previous derivative is approximated using
  /// the other type to get a more stable algorithm.
  using PreviousDerivative = NthDerivative<N - 1,
                                           FunctionType,
                                           NumberType,
                                           typename DifferenceType::OTHER_TYPE>;

  PreviousDerivative previous_derivative;
  NumberType         h;
};

/**
 * Specialization of the NthDerivative class
 * Provides a base case to stop the recursion
 */
template <class FunctionType, class NumberType, class DifferenceType>
class NthDerivative<0u, FunctionType, NumberType, DifferenceType>
{
public:
  /// Constructor.
  NthDerivative(const FunctionType &f, const NumberType &h)
    : function{f}
    , h{h}
  {}

  /// The call operator returns the value of the function itself.
  NumberType
  operator()(const NumberType &x) const {
    return function(x);
  }

private:
  FunctionType function;
  NumberType   h;
};

/**
 * Utility to simplify the creation of a NthDerivative object.
 *
 * Example of usage:
 * @code
 * auto f = [](const double & x) { return x*std::sin(x); };
 * double h = 0.1;
 * auto d = make_forward_derivative<3>(f, h);
 * auto value = d(3.0); // 3rd derivative at 3.0.
 * @endcode
 *
 * @param f A callable function with the rigth signature.
 * @param h The step for computing derivatives.
 */
template <unsigned int N, class FunctionType, class NumberType>
auto
make_forward_derivative(const FunctionType &f, const NumberType &h) {
  return NthDerivative<N, FunctionType, NumberType, Differentiation::FORWARD>{f, h};
}

/**
 * Utility to simplify the creation of a NthDerivative object.
 *
 * Example of usage:
 * @code
 * auto f = [](const double & x) { return x*std::sin(x); };
 * const double h = 0.1;
 * auto d = make_backward_derivative<3>(f, h);
 * @endcode
 * @param f A callable function with the rigth signature.
 * @param h The step for computing derivatives.
 */
template <unsigned int N, class FunctionType, class NumberType>
auto
make_backward_derivative(const FunctionType &f, const NumberType &h) {
  return NthDerivative<N, FunctionType, NumberType, Differentiation::BACKWARD>{f, h};
}

#endif /* DERIVATIVES_HPP */
