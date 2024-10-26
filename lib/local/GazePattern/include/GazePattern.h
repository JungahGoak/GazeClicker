#ifndef GAZE_PATTERN_H
#define GAZE_PATTERN_H

#include "HMM.h"
#include <memory>
#include <vector>

namespace GazePattern {

    std::pair<int, double> find_most_likely_label(
        const std::vector<std::unique_ptr<HMM>>& hmm_models,
        const std::vector<int>& obs, int last_label);

    cv::Point2f predict(
        const std::vector<std::unique_ptr<HMM>>& hmm_models,
        const std::deque<cv::Point2f>& coord_sequence);

}

#endif // GAZE_PATTERN_H