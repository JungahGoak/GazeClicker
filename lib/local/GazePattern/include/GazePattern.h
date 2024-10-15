#ifndef GAZE_PATTERN_H
#define GAZE_PATTERN_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <opencv2/core.hpp>

namespace GazePattern
{

    class HMM {
    public:
        // Constructor
        HMM(int num_states, int num_observations);

        // Baum-Welch algorithm function declaration
        void baum_welch(const std::vector<int>& obs, int n_iters);
            // Functions to update transition and observation probabilities
        void update_transition_and_observation_probabilities(const std::vector<std::vector<double>>& gamma,
                                                         const std::vector<std::vector<std::vector<double>>>& xi,
                                                         const std::vector<int>& obs);
         void print_matrices();
         double calculate_log_likelihood(const std::vector<int>& obs);
         std::pair<int, double> find_most_likely_label(const std::vector<std::unique_ptr<GazePattern::HMM>>& hmm_models, const std::vector<int>& obs);

    private:
        int num_states;
        int num_observations;
        std::vector<std::vector<double>> transition_probabilities; // K x K matrix
        std::vector<std::vector<double>> observation_probabilities; // K x N matrix
        double logsumexp(const double* array, int length) const;
    };

}

#endif // GAZE_PATTERN_H
