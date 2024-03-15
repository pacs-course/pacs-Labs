#include "Agent.hpp"

// pi
#include <numbers>


Agent::Agent( size_t seed, State initial_state, 
                      const Parameters &params):
    m_params(params),
    m_engine(seed),
    m_state(initial_state),
    m_does_sd(false),
    m_t_infection(0),
    m_is_at_pub(false),
    m_t_spent_at_pub(0)
    {
        // generate random initial positions
        std::uniform_real_distribution<double> real_dist(0.0,1.0);

        m_x = real_dist(m_engine) * m_params.domain.domain_size;
        m_y = real_dist(m_engine) * m_params.domain.domain_size;

        std::uniform_int_distribution<unsigned int> 
            randi(1, params.agent.n_timesteps_go_to_pub);

        m_t_go_to_pub = randi(m_engine);

    }

void Agent::move(){

    // method to move the agent

    // decrease time to go to the pub
    if (m_t_go_to_pub >= 0)
        m_t_go_to_pub--;

    // if at the pub, increase time spent at the pub
    if (m_is_at_pub)
        m_t_spent_at_pub++;
    else
    {
    // otherwise, back up the position
        m_x_bak = m_x;
        m_y_bak = m_y;
    }

    // next move
    Move next_move;

    // if is not time to go to the pub
    if ( m_t_go_to_pub > 0 ){
        if (m_does_sd) 
            next_move = Move::Stay;     // we stay 
        else
            next_move = Move::Walk;     // or we walk
    }
    else if (m_is_at_pub)           // if we are a the pub
    {
        if (m_t_spent_at_pub < m_params.agent.n_timesteps_at_pub)
            next_move = Move::Stay;     // we stay at the pub
        else
            next_move = Move::ReturnFromPub;    // or we go back
    }
    else
    {
        next_move = Move::GoToPub;
    } 


    if (next_move == Move::Walk) {
        for (int i=0; i < 1000; ++i){
            const auto [dx, dy] = generate_random_step();
            const auto newx = m_x+dx;
            const auto newy = m_y+dy;

            bool isInside = (newx >= 0.0  && newx <= m_params.domain.domain_size    &&
                             newy >= 0.0  && newy <= m_params.domain.domain_size );

            if (isInside){
                m_x = newx;
                m_y = newy;
                break;
            }

        }
    }
    else if (next_move  == Move::GoToPub) {
        // generate uniform distribution for
        std::uniform_real_distribution<double> rand_x(
                m_params.domain.pub_x - 0.5*m_params.domain.pub_size,
                m_params.domain.pub_x + 0.5*m_params.domain.pub_size);
        std::uniform_real_distribution<double> rand_y(
                m_params.domain.pub_y - 0.5*m_params.domain.pub_size,
                m_params.domain.pub_y + 0.5*m_params.domain.pub_size);
        m_x = rand_x(m_engine);
        m_y = rand_y(m_engine);
        m_t_go_to_pub = -1;
        m_is_at_pub = true;
    }
    else if (next_move == Move::ReturnFromPub)
    {
        m_x = m_x_bak;
        m_y = m_y_bak;
        m_t_go_to_pub    = m_params.agent.n_timesteps_go_to_pub;
        m_t_spent_at_pub = 0;
        m_is_at_pub = false;
    }

    if ( m_state == State::Infected) {
        m_t_infection++;
        if (m_t_infection > m_params.agent.n_timesteps_recover){
            m_state = State::Recovered;
        }
    }
}

std::pair<double, double> Agent::generate_random_step(){
    std::uniform_real_distribution<double> real_dist(0.0, 2.0*std::numbers::pi);
    const auto alpha = real_dist(m_engine);
    const auto dx = std::cos(alpha) * m_params.agent.dr;
    const auto dy = std::sin(alpha) * m_params.agent.dr;

    return {dx,dy};
}



