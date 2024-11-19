#ifndef HMM_H
#define HMM_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <opencv2/core.hpp>

namespace GazePattern {

    class HMM {
        public:
            // Constructor
            HMM(int num_states, int num_observations);

            // Baum-Welch algorithm function declaration
            void baum_welch(const std::vector<int>& obs, int n_iters);

            // Functions to update transition and observation probabilities
            void update_transition_and_observation_probabilities(
                const std::vector<std::vector<double>>& gamma,
                const std::vector<std::vector<std::vector<double>>>& xi,
                const std::vector<int>& obs
            );

            // Print transition and observation matrices
            void print_matrices() const;

            // Calculate log-likelihood of a sequence
            double calculate_log_likelihood(const std::vector<int>& obs);

            // Find the most likely label for a given observation sequence
            static std::pair<int, double> find_most_likely_label(
                const std::vector<std::unique_ptr<HMM>>& hmm_models,
                const std::vector<int>& obs,
                int last_label
            );


        private:
            int num_states; // Number of states in HMM
            int num_observations; // Number of observation symbols
            std::vector<std::vector<double>> transition_probabilities; // Transition probability matrix
            std::vector<std::vector<double>> observation_probabilities; // Observation probability matrix

            // Helper function for numerical stability in log-space
            double logsumexp(const double* array, int length) const;
    };

} // namespace GazePattern

#endif // HMM_H
