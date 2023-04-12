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
// TODO

/// Struct holding the results of the RKF solver.
template </*TODO*/>
struct RKFResult {
  /// Time steps.
  std::vector</*TODO*/> time;

  /// Solutions.
  std::vector</*TODO*/> y;

  /// Error estimate.
  /*TODO*/ error_estimate = 0.0;

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
template </*TODO*/>
class RKF {
 public:
  // define Function type
  /*TODO*/

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
  RKFResult</*TODO*/> solve(TimeType t0,
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

  const ButcherType m_table;
  Function m_function;
};

/*TODO: finish method definition*/::solve(
    TimeType t0,
    TimeType tf,
    const VariableType& y0,
    TimeType h0,
    TimeType tol,
    unsigned int n_max_steps,
    TimeType factor_reduction,
    TimeType factor_expansion) const {

  // Initialize result
  // TODO

  // Reserve some estimated space according to data.
  // TODO

  // Push initial step.
  // TODO

  // Initialize time `t` and step `h` 
  // TODO

  // Loop over each timestep
  for (unsigned iter = 0; iter < n_max_steps; ++iter) {
    // Check if new time step will go past the final time, in case calculate
    // `h` such that `t + h == tf`
    // TODO

    // use `RKFstep`
    // TODO

    // update t and y
    // TODO

    // break if past final time
    // TODO
  }

  return result;
}

/*TODO: finish method definition*/ ::RKFstep(TimeType t,
                                             const VariableType& y,
                                             TimeType h) const {
  // Initialize temporary memory for stages as std::array `K`
  // You need to know how many stages this ButcherType has
  // Remember that ButcherArray has a static constexpr method that can be useful
  // TODO
  
  // for ease of use
  const auto& A = m_table.A;
  const auto& b1 = m_table.b1;
  const auto& b2 = m_table.b2;
  const auto& c = m_table.c;

  // The first step is always an Euler step.
  K[0] = m_function(t, y);
  for (size_t i = 1; i < n_stages; ++i) {
    // t* = t + c_i * h
    // y* = y + \sum_{j = 0}^{i - 1} h * A_{ij} * K_j
    // K_i = f(t*, y*)
    // TODO
  }

  VariableType v1 = y;
  VariableType v2 = y;
  for (unsigned i = 0; i < n_stages; ++i) {
    // v = v + h * b_i * K_i
    // TODO: for both v1, v2 with b1 and b2, respectively
  }

  return std::make_pair(v1, v2);
}

/// Output stream for gnuplot.
/// Possible extension: export also an exact solution, if provided.
template <ScalarOrVector VariableType, Scalar TimeType>
std::ostream& operator<<(std::ostream& out,
                         const RKFResult<VariableType, TimeType>& res) {
  // set output to max precision using std::numeric_limits
  // TODO

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
  // TODO
  out << "\n";

  // write data
  size_t i = 0;
  for (const auto& t : res.time) {
    out << t << "\t";
    // TODO: hadle scalar or vectorial cases
    
    out << "\n";
    ++i;
  }
  return out;
}

#endif /* RKF_HPP */
