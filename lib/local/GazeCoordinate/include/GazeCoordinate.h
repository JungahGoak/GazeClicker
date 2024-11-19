#ifndef GAZE_COORDINATE_H
#define GAZE_COORDINATE_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <opencv2/core.hpp>
#include <chrono>
#include <mutex>

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
            bool isPredictMode = false;  // 초기 상태는 learn mode

            void updateSlope(cv::Point2f lastCoord, cv::Point2f clickCoord);
            void updateSequence(cv::Point2f newCoord);
            double intercept; // slope 초기 절편 값

            bool getIsDwellTime();
            void setIsDwellTime(bool value, std::chrono::steady_clock::time_point startTime);

            bool getIsClickTrigger();
            void setIsClickTrigger(bool value);

            bool getIsPredictMode();
            void setIsPredictMode(bool value);
            
            std::chrono::steady_clock::time_point startedDwellTime;
            cv::Point2f click_coord;

        private:

            // 내부 멤버 변수와 뮤텍스
            bool isDwellTime = false;
            bool isClickTrigger = false;

            std::mutex dwellTimeMutex;  // DwellTime용 뮤텍스
            std::mutex clickTriggerMutex;  // ClickTrigger용 뮤텍스
            std::mutex isPredictModeMutex;

            double correction_rate;        // slope 보정 비율
    };
}

#endif // GAZE_COORDINATE_H