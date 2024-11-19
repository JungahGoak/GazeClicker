#include "GazePattern.h"
#include "GazeClickerConfig.h"
#include "MappingScreen.h"
#include <iostream>
#include <cmath>
#include <limits>

namespace GazePattern {

cv::Point2f predict(
    const std::vector<std::unique_ptr<HMM>>& hmm_models,
    const std::deque<cv::Point2f>& coord_sequence){
    
    // HMM을 사용해 예측 결과 얻기
    std::vector<int> directionSequence = MappingScreen::convertCoordinatesToDirections(coord_sequence);
    auto hmm_result = find_most_likely_label(hmm_models, directionSequence, MappingScreen::getLabelFromCoord(coord_sequence[-1]));

    if (hmm_result.first != -1 and hmm_result.second >= 0.95) {
        std::cout << "======> [예측] HMM 예측: " << hmm_result.first << " with log-likelihood: " << hmm_result.second << std::endl;
        cv::Point2f center = MappingScreen::getLabelCenter(hmm_result.first);
        return center;
    } else {
        std::cout << "======> [예측] 일반 예측: " << MappingScreen::getLabelFromCoord(coord_sequence[-1]) << " | HMM 예측: " << hmm_result.first << " with log-likelihood: " << hmm_result.second << std::endl;
        return coord_sequence[-1];
    }
}

std::pair<int, double> find_most_likely_label(
    const std::vector<std::unique_ptr<HMM>>& hmm_models,
    const std::vector<int>& obs, int last_label) {
    
    int best_label = -1;
    double max_log_likelihood = -std::numeric_limits<double>::infinity();
    int start = std::max(0, last_label - GRID_SIZE - 1);
    int end = std::min(static_cast<int>(hmm_models.size()), last_label + GRID_SIZE + 1);

    for (int label = start; label < end; ++label) {
        if (hmm_models[label]) {
            double log_likelihood = hmm_models[label]->calculate_log_likelihood(obs);
            std::cout << "Label " << label << " log-likelihood: " << log_likelihood << std::endl;

            if (log_likelihood > max_log_likelihood) {
                max_log_likelihood = log_likelihood;
                best_label = label;
            }
        }
    }

    return {best_label, exp(max_log_likelihood)};
}

} // namespace GazePattern
