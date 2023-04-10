#ifndef RKF_HPP
#define RKF_HPP

#include "ButcherRKF.hpp"
#include "RKFTraits.hpp"

#include <iostream>
#include <limits>
#include <vector>

template <typename T>
concept IsButcherArray =
    instance_of_non_type<ButcherArray,
                         decltype(base_of(&T::A))>  // the base class of T is
                                                    // ButcherArray<N>
    && !
std::is_same<T, decltype(base_of(&T::A))>::value;  // T is not a ButcherArray<N>

/// Struct holding the results of the RKF solver.
template <ScalarOrVector VariableType>
struct RKFResult {
  /// Time steps.
  std::vector<double> time;

  /// Solutions.
  std::vector<VariableType> y;

  /// Error estimate.
  double error_estimate = 0.0;

  /// Failure.
  bool failed = false;

  /// Number of time step expansions.
  unsigned int expansions = 0;

  /// Number of time step reductions.
  unsigned int reductions = 0;
};

/// Runge-Kutta-Fehlberg solver class.
template <IsButcherArray ButcherType,
          ScalarOrVector VariableType,
          Scalar TimeType = double>
class RKF {
 public:
  using Function = std::function<VariableType(TimeType, const VariableType&)>;

  /// Default constructor.
  RKF() = default;

  /// Constructor.
  RKF(const Function& function_) : m_function(function_) {}

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
  RKFResult<VariableType> solve(TimeType t0,
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
      -> std::pair<VariableType, VariableType>;

  inline static auto norm(const VariableType& x) {
    if constexpr (Scalar<VariableType>) {
      // only issue here is that if sizeof(VariableType) < 8
      // we are a little bit inefficient since we are passing by
      // reference (i.e. copy the address of the variable which has size
      // 64bit on most architectures) instead of passing by copy
      return std::abs(x);
    } else {
      return x.norm();
    }
  }

  const ButcherType m_table;
  Function m_function;
};

template <IsButcherArray ButcherType,
          ScalarOrVector VariableType,
          Scalar TimeType>
RKFResult<VariableType> RKF<ButcherType, VariableType, TimeType>::solve(
    TimeType t0,
    TimeType tf,
    const VariableType& y0,
    TimeType h0,
    TimeType tol,
    unsigned int n_max_steps,
    TimeType factor_reduction,
    TimeType factor_expansion) const {
  RKFResult<VariableType> result;

  auto& [time, y, error_estimate, failed, expansions, reductions] = result;

  error_estimate = 0.0;
  failed = false;
  expansions = 0;
  reductions = 0;

  // Reserve some estimated space according to data.
  const size_t n_expected_steps = std::min(
      std::max(1u, 1 + static_cast<unsigned int>((tf - t0) / h0)), n_max_steps);

  time.reserve(n_expected_steps);
  y.reserve(n_expected_steps);

  // Push initial step.
  time.push_back(t0);
  y.push_back(y0);

  bool rejected = false;

  // Check that the time step does not become ridiculously small.
  const auto time_span = tf - t0;
  auto t = t0;

  const auto h_min =
      100 * time_span * std::numeric_limits<decltype(time_span)>::epsilon();
  auto h = std::max(h0, h_min);

  VariableType y_curr = y0;

  unsigned int iter = 0;

  for (; iter < n_max_steps; ++iter) {
    // Low precision solution.
    VariableType y_low;

    // High precision solution.
    VariableType y_high;

    // Check if new time step will go past the final time.
    if (t + h < tf) {
      std::tie(y_low, y_high) = RKFstep(t, y_curr, h);
    } else {
      h = tf - t;  // Truncate time step.

      if (h < h_min) {
        // Step ridiculously small. But we are at the end, so we may stop
        // here.
        y_low = y_curr;
        y_high = y_curr;
      } else
        std::tie(y_low, y_high) = RKFstep(t, y_curr, h);
    }

    const auto error = norm(y_low - y_high);

    if (error <= tol * h / time_span) {
      t += h;

      time.push_back(t);
      y.push_back(y_high);

      y_curr = y_high;

      error_estimate += error;

      if (!rejected && (t < tf)) {
        h *= factor_expansion;
        ++expansions;
      }

      rejected = false;
    } else {
      rejected = true;
      ++reductions;

      h *= factor_reduction;
      h = std::max(h, h_min);
    }

    if (t >= tf)
      break;
  }

  if ((iter >= n_max_steps) && (t < tf))
    failed = true;

  return result;
}

template <IsButcherArray ButcherType,
          ScalarOrVector VariableType,
          Scalar TimeType>
std::pair<VariableType, VariableType>
RKF<ButcherType, VariableType, TimeType>::RKFstep(TimeType t,
                                                  const VariableType& y,
                                                  TimeType h) const {
  constexpr auto n_stages = ButcherType::n_stages();

  std::array<VariableType, n_stages> K;

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
  for (unsigned int i = 0; i < n_stages; ++i) {
    v1 += h * K[i] * b1[i];
    v2 += h * K[i] * b2[i];
  }

  return std::make_pair(v1, v2);
}

/// Output stream for gnuplot.
/// Possible extension: export also an exact solution, if provided.
template<ScalarOrVector VariableType>
std::ostream& operator<<(std::ostream& out, const RKFResult<VariableType>& res) {
  out << "# Number ot time steps: " << res.time.size() << "\n"
      << "# Number of reductions: " << res.reductions << "\n"
      << "# Number of expansions: " << res.expansions << "\n"
      << "# Error estimate: " << res.error_estimate << "\n";

  auto h_min = res.time[1] - res.time[0];
  auto h_max = h_min;

  for (unsigned int i = 0; i < res.time.size() - 1; ++i) {
    const auto dt = res.time[i + 1] - res.time[i];
    h_max = std::max(h_max, dt);
    h_min = std::min(h_min, dt);
  }

  out << "# h_min: " << h_min << ", h_max: " << h_max << "\n";

  out << "t\t";
  if constexpr (Scalar<VariableType>)
    out << "y";
  else {  // if constexpr (std::is_same_v<ProblemType, RKFType::Vector>)
    for (int k = 0; k < res.y[0].size(); ++k)
      out << "y[" << k << "]\t";
  }

  out << "\n";

  size_t i = 0;
  for (const auto& t : res.time) {
    out << "  " << t << "\t";
    const auto& yy = res.y[i];

    if constexpr (Scalar<VariableType>) {
      out << res.y[i];
    } else  // if constexpr (std::is_same_v<ProblemType, RKFType::Vector>)
    {
      for (int k = 0; k < yy.size(); ++k)
        out << yy[k] << "\t";
    }

    out << "\n";
    ++i;
  }
  return out;
}

#endif /* RKF_HPP */
