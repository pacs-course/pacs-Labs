#ifndef RKF_HPP
#define RKF_HPP

#include <GetPot>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "ButcherRKF.hpp"
#include "ButcherTraits.hpp"
#include "RKFTraits.hpp"

/// Static function to evaluate the norm of a VariableType
template <ScalarOrVector VariableType>
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

template <ScalarOrVector VariableType>
using ErrorType =
    std::invoke_result_t<decltype(norm<VariableType>), VariableType>;

/// Struct holding the results of the RKF solver.
template <ScalarOrVector VariableType, Scalar TimeType = double>
struct RKFResult {
  /// Time steps.
  std::vector<TimeType> time;

  /// Solutions.
  std::vector<VariableType> y;

  /// Error estimate.
  ErrorType<VariableType> error_estimate = 0.0;

  /// Failure.
  bool failed = false;

  /// Number of time step expansions.
  unsigned int expansions = 0;

  /// Number of time step reductions.
  unsigned int reductions = 0;
};

/// Runge-Kutta-Fehlberg option class.
template <ScalarOrVector VariableType, Scalar TimeType = double>
class RKFOptions {
 public:
  // Make the RKF class a friend for all its template combinations,
  // so they can access private members of this class.
  template <class, ScalarOrVector, Scalar>
  friend class RKF;

  RKFOptions() = default;

  RKFOptions(const std::string& filename) { parse_from_file(filename); }

 private:
  void parse_from_file(const std::string& filename) {
    GetPot datafile(filename.c_str());

    const int size = datafile("size", 1);

    t0 = datafile("t0", 0.);
    tf = datafile("tf", 1.);

    if constexpr (Scalar<VariableType>) {
      y0 = datafile("y0", 0.);
    } else {  // if constexpr (Vector<VariableType>)
      // if the variable has static storage (e.g. std::array) we do not need to
      // resize it another option is to check if VariableType has a `resize`
      // method here an example https://stackoverflow.com/a/87846
      if constexpr (is_constexpr_friendly([] { VariableType{}.size(); })) {
        // storage is static, check is is the same size read from file
        if (size != static_cast<int>(y0.size())) {
          throw std::runtime_error(
              "Size read from parameter is different from VariableType (which "
              "has static storage)");
        }
      } else {  // storage is dynamic, resize
        y0.resize(size);
      }
      // Read y0 as a list, but the size must be known.
      for (int i = 0; i < size; ++i) y0[i] = datafile("y0", 0.0, i);
    }

    h0 = datafile("h0", 0.1);

    tolerance = datafile("tolerance", 1e-6);
    n_max_steps = datafile("n_max_steps", 1e3);

    factor_reduction = datafile("factor_reduction", 0.95);
    factor_expansion = datafile("factor_expansion", 2);
  }

  /// Initial time.
  TimeType t0;
  /// Final time.
  TimeType tf;

  /// Initial condition.
  VariableType y0;

  /// Initial time step.
  TimeType h0;

  /// Desired tolerance on error.
  ErrorType<VariableType> tolerance = 1e-6;
  /// Safeguard to avoid too many steps.
  unsigned int n_max_steps = 1e3;

  /// Multiplication factor for time step reduction.
  TimeType factor_reduction = 0.95;
  /// Multiplication factor for time step expansion.
  TimeType factor_expansion = 2;
};

/// Runge-Kutta-Fehlberg solver class.
// Can we create a "IsButcherArray" concept for "ButcherType"?
// Yes, see "ButcherTraits.hpp", however it is non-trivial.
// Concepts are a way to write code that is easier to read and debug.
// You have to evaluate the trade-off between using
// a plain "typename/class" vs defining an ad-hoc "concept".
template <class ButcherType, ScalarOrVector VariableType,
          Scalar TimeType = double>
class RKF {
 public:
  using Function = std::function<VariableType(TimeType, const VariableType&)>;
  using Options = RKFOptions<VariableType, TimeType>;

  /// Default constructor.
  RKF() = default;

  /// Constructor.
  RKF(const Function& function_, const Options& options_ = Options())
      : m_options(options_), m_function(function_) {}

  /// Set options.
  void set_options(const Options& options_) { m_options = options_; }

  /// Set the forcing term.
  void set_function(const Function& function_) { m_function = function_; }

  /**
   * Solve problem.
   */
  RKFResult<VariableType, TimeType> solve() const;

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

  Options m_options;
  const static inline ButcherType m_table;
  Function m_function;
};

template <class ButcherType, ScalarOrVector VariableType, Scalar TimeType>
RKFResult<VariableType, TimeType>
RKF<ButcherType, VariableType, TimeType>::solve() const {
  const auto& [t0, tf, y0, h0, tolerance, n_max_steps, factor_reduction,
               factor_expansion] = m_options;

  RKFResult<VariableType, TimeType> result;
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
  auto h = h0;

  // declare outside because we need it after the loop
  unsigned int iter = 0;
  for (; iter < n_max_steps; ++iter) {
    // Check if new time step will go past the final time.
    if (t + h > tf) {
      h = tf - t;  // Truncate time step.
    }
    // Low and high precision solution.
    const auto [y_low, y_high] = RKFstep(t, y.back(), h);

    const auto error = norm<VariableType>(y_low - y_high);

    if (error <= tolerance * h / time_span) {
      t += h;

      time.push_back(t);
      y.push_back(y_high);

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
    }

    if (t >= tf) break;
  }

  if ((iter >= n_max_steps) && (t < tf)) failed = true;

  return result;
}

template <class ButcherType, ScalarOrVector VariableType, Scalar TimeType>
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
    for (unsigned int j = 0; j < i; ++j) value += h * A[i][j] * K[j];

    K[i] = m_function(time, value);
  }

  VariableType v1 = y;
  VariableType v2 = y;
  for (unsigned int i = 0; i < n_stages; ++i) {
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
  // set output to max precision
  out << std::scientific;
  if constexpr (Scalar<VariableType>)
    out << std::setprecision(std::numeric_limits<VariableType>::digits10 + 1);
  else {
    using ScalarType = std::decay_t<decltype(res.y[0][0])>;
    out << std::setprecision(std::numeric_limits<ScalarType>::digits10 + 1);
  }

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

  // write header
  out << "t\t";
  if constexpr (Scalar<VariableType>)
    out << "y";
  else {
    for (decltype(res.y[0].size()) k = 0; k < res.y[0].size(); ++k)
      out << "y[" << k << "]\t";
  }
  out << "\n";

  // write data
  size_t i = 0;
  for (const auto& t : res.time) {
    out << t << "\t";
    if constexpr (Scalar<VariableType>) {
      out << res.y[i];
    } else {
      for (decltype(res.y[i].size()) k = 0; k < res.y[i].size(); ++k)
        out << res.y[i][k] << "\t";
    }
    out << "\n";
    ++i;
  }
  return out;
}

#endif /* RKF_HPP */
