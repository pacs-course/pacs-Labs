#include "Agent.hpp"

// pi
#include <numbers>


Agent::Agent( size_t seed, State initial_state, 
                      const Parameters &params):
    m_params(params),
    m_engine(seed),
    m_state(initial_state){

        // generate random initial positions
        std::uniform_real_distribution<double> real_dist(0.0,1.0);

        m_x = real_dist(m_engine) * m_params.domain.domain_size;
        m_y = real_dist(m_engine) * m_params.domain.domain_size;

    }

void Agent::move(){
    // we try to move until the next step is inside the domain
    //
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

std::pair<double, double> Agent::generate_random_step(){
    std::uniform_real_distribution<double> real_dist(0.0, 2.0*std::numbers::pi);
    const auto alpha = real_dist(m_engine);
    const auto dx = std::cos(alpha) * m_params.agent.dr;
    const auto dy = std::sin(alpha) * m_params.agent.dr;

    return {dx,dy};
}



