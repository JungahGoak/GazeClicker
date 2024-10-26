#include "Click.h"
#include <iostream>

namespace Utilities {

// 응시 고정 여부 확인
bool Click::isFixation(const cv::Point2f& currentCoord, const std::deque<cv::Point2f>& coord_sequence) {
    if (coord_sequence.empty()) return false;

    const auto& lastCoord = coord_sequence.back();
    return cv::norm(currentCoord - lastCoord) <= fixation_distance_threshold;
}

// 응시 시간 측정 시작
void Click::startFixation() {
    fixation_start = std::chrono::steady_clock::now();
    isCounting = true;
    std::cout << "Fixation started." << std::endl;
}

// 응시 시간 업데이트 및 클릭 이벤트 처리
bool Click::updateFixation(const cv::Point2f& newCoord, const std::deque<cv::Point2f>& coord_sequence) {
    if (isFixation(newCoord, coord_sequence)) {
        if (!isCounting) {
            startFixation();  // 응시 시간 측정 시작
        } else {
            // 응시 시간 확인
            auto now = std::chrono::steady_clock::now();
            double duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - fixation_start).count() / 1000.0;

            std::cout << "Fixation duration: " << duration << " seconds" << std::endl;

            if (duration >= fixation_threshold) {
                std::cout << "Fixation confirmed. Triggering click event." << std::endl;
                triggerClickEvent();  // 클릭 이벤트 실행
                isCounting = false;  // 시간 측정 종료
                return true;
            }
        }
    } else {
        // 고정 해제 시 타이머 초기화
        isCounting = false;
        std::cout << "Fixation lost. Timer reset." << std::endl;
    }
    return false;
}

// 클릭 이벤트 발생 함수
void Click::triggerClickEvent() {
    std::cout << "Click event triggered!" << std::endl;
}

}  // namespace Utilities
