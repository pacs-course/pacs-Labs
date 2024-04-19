#ifndef RKF_HPP
#define RKF_HPP

#include "ButcherRKF.hpp"
#include "ButcherTraits.hpp"
#include "RKFTraits.hpp"

#include <iostream>
#include <limits>
#include <vector>
#include <iomanip>
#include <functional>

/// Static function to evaluate the norm of a VariableType
template <ScalarOrVector VariableType>
inline static auto norm(const VariableType& x) {
  if constexpr (Scalar<VariableType>) {
    return std::abs(x);
  }
  else {
    return x.norm();
  }
}

/// Struct holding the results of the RKF solver.
template <ScalarOrVector VariableType, Scalar TimeType>
struct RKFResult {
  using ErrorType = std::invoke_result_t<decltype(norm<VariableType>), VariableType>;
  /// Time steps.
  std::vector<TimeType> time;

  /// Solutions.
  std::vector<VariableType> y;

  /// Error estimate.
  ErrorType error_estimate = 0.0;

  /// Failure.
  bool failed = false;

  /// Number of time step expansions.
  unsigned int expansions = 0;

  /// Number of time step reductions.
  unsigned int reductions = 0;
};

/// Runge-Kutta-Fehlberg solver class.
// Can we create a "IsButcherArray" concept for "ButcherType"?
// Yes, see "ButcherTraits.hpp", however it is non-trivial.
// Concepts are a way to write code that is easier to read and debug.
// You have to evaluate the trade-off between using
// a plain "typename/class" vs defining an ad-hoc "concept".
template <class ButcherType, ScalarOrVector VariableType, Scalar TimeType = double>
class RKF {
public:
  // define Function type
  using Function = std::function<VariableType(TimeType, const VariableType&)>;

  /// Default constructor.
  RKF() = default;

  /// Constructor.
  RKF(const Function& function_): m_function(function_) {}

  /// Set the forcing term.
  void set_function(const Function& function_) { m_function = function_; }

  /**
   * Solve problem.
   *
   * @param[in] t0               Initial time.
   * @param[in] tf               Final time.
   * @param[in] y0               Initial condition.
   * @param[in] h0               Initial time step.
   * @param[in] tol              Desired tolerance on error.
   * @param[in] n_max_steps      Safeguard to avoid too many steps.
   * @param[in] factor_reduction Multiplication factor for time step reduction.
   * @param[in] factor_expansion Multiplication factor for time step expansion.
   */
  RKFResult<VariableType, TimeType> solve(TimeType t0,
    TimeType tf,
    const VariableType& y0,
    TimeType h0,
    TimeType tol,
    unsigned int n_max_steps,
    TimeType factor_reduction = 0.95,
    TimeType factor_expansion = 2) const;

private:
  /**
   * Function for a single step. It is private since it is used only internally.
   *
   * @param[in] t Time.
   * @param[in] y Solution at time t.
   * @param[in] h Time step.
   *
   * @return The low- and high-order solutions.
   */
  auto RKFstep(TimeType t, const VariableType& y, TimeType h) const
    ->std::pair<VariableType, VariableType>;

  const ButcherType m_table;
  Function m_function;
};

template<class ButcherType, ScalarOrVector VariableType, Scalar TimeType>
RKFResult<VariableType, TimeType> RKF<ButcherType, VariableType, TimeType>::solve(
  TimeType t0,
  TimeType tf,
  const VariableType& y0,
  TimeType h0,
  TimeType tol,
  unsigned int n_max_steps,
  TimeType factor_reduction,
  TimeType factor_expansion) const {

  // Initialize result
  RKFResult<VariableType, TimeType> result;
  auto& [time, y, error_estimate, failed, expansions, reductions] = result;

  // Reserve some estimated space according to data.
  const size_t n_expected_steps = std::min(
    std::max(1u, 1 + static_cast<unsigned int>((tf - t0) / h0)), n_max_steps);

  time.reserve(n_expected_steps);
  y.reserve(n_expected_steps);

  // Push initial step.
  time.push_back(t0);
  y.push_back(y0);

  // Initialize time `t` and step `h` 
  auto t = t0;
  auto h = h0;

  // TODO: add `rejected` variable, default to false

  // Loop over each timestep
  for (unsigned iter = 0; iter < n_max_steps; ++iter) {
    // Check if new time step will go past the final time, in case calculate
    // `h` such that `t + h == tf`
    if (t + h > tf) {
      h = tf - t;
    }

    // use `RKFstep`
    const auto [y_low, y_high] = RKFstep(t, y.back(), h);

    // update t and y
    t += h;
    y.push_back(y_high);
    time.push_back(t);

    // TODO: implement timestep adaptivity
    // 1. compute the error as the norm of the difference between y_low and y_high
    //      use the norm function we implemented above
    // 2. if the error is small (error <= tol * h / time_span), then
    //       a. update t and y as done before (remove the update above)
    //       b. update `error_estimate` adding current `error`
    //       c. if last step was not `rejected` and `t < tf`, then 
    //           - we use a larger step (multiply `h` by `factor_expansion`)
    //           - increment the count of `expansions`
    //       d. set `rejected` to false since we accepted this step
    // 2. else the error is large, we cannot accept the step
    //       a. set `rejected` to true since the error is large
    //       b. we use a smaller step (multiply `h` by `factor_reduction`)
    //       c. increment the count of `reductions`

    // break if past final time
    if (t >= tf)
      break;
  }

  return result;
}

template<class ButcherType, ScalarOrVector VariableType, Scalar TimeType>
std::pair<VariableType, VariableType> RKF<ButcherType, VariableType, TimeType>::RKFstep(TimeType t,
  const VariableType& y,
  TimeType h) const {
  // Initialize temporary memory for stages as std::array `K`
  // You need to know how many stages this ButcherType has
  // Remember that ButcherArray has a static constexpr method that can be useful
  constexpr auto n_stages = ButcherType::n_stages();
  std::array<VariableType, n_stages> K;

  // for ease of use
  const auto& A = m_table.A;
  const auto& b1 = m_table.b1;
  const auto& b2 = m_table.b2;
  const auto& c = m_table.c;

  // The first step is always an Euler step.
  K[0] = m_function(t, y);
  for (size_t i = 1; i < n_stages; ++i) {
    const auto time = t + c[i] * h;

    VariableType value = y;
    for (unsigned int j = 0; j < i; ++j)
      value += h * A[i][j] * K[j];

    K[i] = m_function(time, value);
  }

  VariableType v1 = y;
  VariableType v2 = y;
  for (unsigned i = 0; i < n_stages; ++i) {
    v1 += h * b1[i] * K[i];
    v2 += h * b2[i] * K[i];
  }

  return std::make_pair(v1, v2);
}

/// Output stream for gnuplot.
/// Possible extension: export also an exact solution, if provided.
template <ScalarOrVector VariableType, Scalar TimeType>
std::ostream& operator<<(std::ostream& out,
  const RKFResult<VariableType, TimeType>& res) {
  // set output to max precision using std::numeric_limits
  out << std::scientific;
  if constexpr (Scalar<VariableType>)
    out << std::setprecision(std::numeric_limits<VariableType>::digits10 + 1);
  else {
    using ScalarType = std::decay_t<decltype(res.y[0][0])>;
    out << std::setprecision(std::numeric_limits<ScalarType>::digits10 + 1);
  }

  // write statistics
  out << "# Number ot time steps: " << res.time.size() << "\n"
    << "# Number of reductions: " << res.reductions << "\n"
    << "# Number of expansions: " << res.expansions << "\n"
    << "# Error estimate: " << res.error_estimate << "\n";

  auto h_min = res.time[1] - res.time[0];
  auto h_max = h_min;

  for (size_t i = 0; i < res.time.size() - 1; ++i) {
    const auto dt = res.time[i + 1] - res.time[i];
    h_max = std::max(h_max, dt);
    h_min = std::min(h_min, dt);
  }

  out << "# h_min: " << h_min << ", h_max: " << h_max << "\n";

  // write header, "t,y" if scalar, "t,y[0],y[1],..." if vectorial
  out << "t\t";
  if constexpr (Scalar<VariableType>)
    out << "y";
  else {
    for (decltype(res.y[0].size()) k = 0; k < res.y[0].size(); ++k)
      out << "y[" << k << "]\t";
  }  out << "\n";

  // write data
  size_t i = 0;
  for (const auto& t : res.time) {
    out << t << "\t";
    if constexpr (Scalar<VariableType>) {
      out << res.y[i];
    }
    else {
      for (decltype(res.y[i].size()) k = 0; k < res.y[i].size(); ++k)
        out << res.y[i][k] << "\t";
    }
    out << "\n";
    ++i;
  }
  return out;
}

#endif /* RKF_HPP */
