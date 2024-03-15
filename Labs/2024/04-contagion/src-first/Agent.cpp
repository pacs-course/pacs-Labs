#include "Agent.hpp"


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
    for (int i=0; i < 1000; ++i){
        // TODO
        // try to move. If not inside the domain, generate a new move
    }
}

std::pair<double, double> Agent::generate_random_step(){
    // TODO: 
    // generate the random angle and step size
}
