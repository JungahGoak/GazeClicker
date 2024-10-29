#ifndef GAZE_COORDINATE_H
#define GAZE_COORDINATE_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <opencv2/core.hpp>
#include <chrono>

#include "GazePattern.h"
#include "GazeClickerConfig.h"
#include "UI.h"

namespace GazeCoordinate
{
    class GazeCoordinate {
        public:
            // Constructor
            GazeCoordinate();

            Utilities::UI ui;

            std::deque<cv::Point2f> coord_sequence;
            std::vector<std::unique_ptr<GazePattern::HMM>> hmm_models;
            std::vector<double> slopes;

            void updateSlope(cv::Point2f lastCoord, cv::Point2f clickCoord);
            void updateSequence(cv::Point2f newCoord);
            double intercept; // slope 초기 절편 값

            // dwell time
            bool statusDwellTime;
            std::chrono::steady_clock::time_point startedDwellTime;

            // click popup
            bool isClickTrigger; // ClickTrigger 실행 시 사용할 변수

        private:          
            double correction_rate;        // slope 보정 비율
    };
}

#endif // GAZE_COORDINATE_H