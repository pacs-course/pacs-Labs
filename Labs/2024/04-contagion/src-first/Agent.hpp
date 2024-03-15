#ifndef AGENT_HPP
#define AGENT_HPP

#include "Parameters.hpp"

// random library for random number generator
#include <random>

enum class State {
    Susceptible,
    Infected,
    Recovered
};

class Agent
{
public:
  // constructor takes:
  // seed for number repreducibility of random numbers
  // state
  // parameters
  Agent( size_t seed, State state, const Parameters &param);

  // getter methods
  double x() const { return m_x;};
  double y() const { return m_y;};
  State state() const { return m_state;};
  bool susceptible() const { return m_state == State::Susceptible;};
  bool    infected() const { return m_state == State::Infected;};
  bool   recovered() const { return m_state == State::Recovered;};

  // setter methods
  void set_state(State s){ m_state = s;};

  // move agent
  void move(); // TODO

protected:
  std::pair<double, double> generate_random_step();  // TODO

  const Parameters & m_params;
  double m_x;
  double m_y;
  State m_state;

  // member engine
  std::default_random_engine m_engine;

};

#endif /* AGENT_HPP */
