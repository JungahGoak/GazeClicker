#ifndef CLICK_H
#define CLICK_H

#include <vector>
#include <deque>
#include <opencv2/core/core.hpp>
#include <chrono> 

namespace Utilities {

    class Click {
    private:
        std::chrono::steady_clock::time_point fixation_start;  // 응시 시작 시간
        bool isCounting = false;  // 응시 시간 측정 중인지 여부
        double fixation_threshold = 2.0;  // 응시 임계 시간 (초)
        double fixation_distance_threshold = 10.0;  // 고정 여부를 판단하는 거리 임계값

    public:
        // 응시 고정 여부 확인
        bool isFixation(const cv::Point2f& currentCoord, const std::deque<cv::Point2f>& coord_sequence);

        // 응시 시간 측정 시작
        void startFixation();

        // 응시 시간 업데이트 및 클릭 이벤트 처리
        bool updateFixation(const cv::Point2f& newCoord, const std::deque<cv::Point2f>& coord_sequence);

        // 클릭 이벤트 발생
        void triggerClickEvent();
    };

}  // namespace Utilities

#endif