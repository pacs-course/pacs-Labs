#include "Contagion.hpp"

// header for file streams
#include <fstream>

// header for string streams
#include <sstream>

// header for io manipulation
#include <iomanip>

Contagion::Contagion(const std::string &filename)
  : m_params(filename),
    m_time(m_params.contagion.n_timesteps+1),
    m_n_infected(m_params.contagion.n_timesteps+1),
    m_n_recovered(m_params.contagion.n_timesteps+1),
    m_n_susceptible(m_params.contagion.n_timesteps+1)
    {
        std::default_random_engine engine(m_params.seed);
        m_agents.reserve(m_params.contagion.n_agents);

        for (unsigned i = 0; i < m_params.contagion.n_init_infected; ++i){
            m_agents.emplace_back(engine(), State::Infected, m_params);
        }
        for (unsigned i = 0; i < m_params.contagion.n_agents - 
                                 m_params.contagion.n_init_infected; ++i){
            m_agents.emplace_back(engine(), State::Susceptible, m_params);
        }

    }

void Contagion::run() {

    // first time step:
    m_time[0] = 0.0;
    // initial values
    m_n_susceptible[0] = m_params.contagion.n_agents - m_params.contagion.n_init_infected;
    m_n_infected[0]    = m_params.contagion.n_init_infected;
    m_n_recovered[0]   = 0;

    for (unsigned int step = 1; step < m_params.contagion.n_timesteps; ++step)
    {
        // time in days
        m_time[step] = static_cast<double>(step) / m_params.contagion.n_timesteps_per_day;

        // move each agents
        for (auto &a : m_agents){
            a.move();
        }

        // check the distance between all the agents
        for (size_t i=0; i < m_agents.size()-1; ++i){
            for (size_t j=i+1; j < m_agents.size(); ++j){
                if ( (m_agents[i].susceptible() && m_agents[j].infected() ) ||
                     (m_agents[j].susceptible() && m_agents[i].infected() ) )
                    {
                        double dx = m_agents[i].x() - m_agents[j].x();
                        double dy = m_agents[i].y() - m_agents[j].y();

                        if ( (dx*dx + dy*dy) <= m_params.contagion.r_infection_2){
                            m_agents[i].set_state(State::Infected);
                            m_agents[j].set_state(State::Infected);
                        }
                    }
            }
        }

        //     - update each single agent SIR
        for (const auto & a: m_agents) {
            m_n_susceptible[step] += a.susceptible();
            m_n_infected[step] += a.infected();
            m_n_recovered[step] += a.recovered();
        }

        if (m_params.dump && (step % m_params.dump == 0))
        {
            // get the padded filename
            std::ostringstream oss;

            oss << "snapshot." << std::setfill('0') << std::setw(5) 
                              << step << ".csv";

            // output the position
            Contagion::output_position(oss.str());
        }
    }
    output_results();
}

void Contagion::output_results() const {
    std::ofstream file("output.csv", std::ofstream::out);
    
    // print header file
    file << "time,n_susceptible,n_infected,n_recovered\n";

    for (size_t i=0; i < m_time.size(); ++i){
        file << m_time[i] << "," << m_n_susceptible[i] <<  "," 
                               << m_n_infected[i] << "," 
                               << m_n_recovered[i] << "\n";
    }
    file.close();
}

// this method should be further checked
void Contagion::output_position( const std::string & filename) const {
    std::ofstream file(filename, std::ofstream::out);
    file << "x,y,state\n" ;
    for (const auto & a: m_agents){
        file << a.x() << "," << a.y() << "," << static_cast<int> (a.state()) << "\n";
    }
    file.close();
}



