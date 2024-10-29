#include "Click.h"
#include "UI.h"
#include "GazeCoordinate.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <thread>
#include <chrono>  // std::chrono 사용

namespace DwellClick {

// 응시 고정 여부 확인
bool Click::isFixation(const cv::Point2f& currentCoord, const std::deque<cv::Point2f>& coord_sequence) {
    if (coord_sequence.empty()) return false;

    const auto& lastCoord = coord_sequence.back();
    return cv::norm(currentCoord - lastCoord) <= fixation_distance_threshold;
}

// 응시 시간 측정 시작
void Click::startFixation(GazeCoordinate::GazeCoordinate& gazeCoord) {
    gazeCoord.startedDwellTime = std::chrono::steady_clock::now();
    gazeCoord.statusDwellTime = true;
    std::cout << "Fixation started." << std::endl;
}

// 응시 시간 업데이트 및 클릭 이벤트 처리
bool Click::updateFixation(const cv::Point2f& cur_screen_coord, GazeCoordinate::GazeCoordinate& gazeCoord) {
    if (isFixation(cur_screen_coord, gazeCoord.coord_sequence)) {
        if (!gazeCoord.statusDwellTime) {
            startFixation(gazeCoord);  // 응시 시간 측정 시작
        } else {
            // 응시 시간 확인
            auto now = std::chrono::steady_clock::now();
            double duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - gazeCoord.startedDwellTime).count() / 1000.0;

            std::cout << "Fixation duration: " << duration << " seconds" << std::endl;

            if (duration >= fixation_threshold) {
                std::cout << "Fixation confirmed. Triggering click event." << std::endl;

                // triggerClickEvent를 새로운 스레드에서 실행
                std::thread clickThread(&Click::triggerClickEvent, this, cur_screen_coord, std::ref(gazeCoord));
                clickThread.detach();  // 스레드를 분리
                //triggerClickEvent(cur_screen_coord, gazeCoord);
                
                gazeCoord.statusDwellTime = false;  // 시간 측정 종료
                return true;
            }
        }
    } else {
        // 고정 해제 시 타이머 초기화
        gazeCoord.statusDwellTime = false;
        gazeCoord.startedDwellTime = std::chrono::steady_clock::time_point();
        std::cout << "Fixation lost. Timer reset." << std::endl;
    }
    return false;
}

// 클릭 이벤트 발생 함수
void Click::triggerClickEvent(const cv::Point2f cur_screen_coord, GazeCoordinate::GazeCoordinate& gazeCoord) {
    std::cout << "Click event triggered!" << cur_screen_coord << std::endl;
    gazeCoord.isClickTrigger = true;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 2초 후 isClickTrigger를 false로 설정
    gazeCoord.isClickTrigger = false;
    gazeCoord.startedDwellTime = std::chrono::steady_clock::time_point();
    gazeCoord.statusDwellTime = false;
    std::cout << "isClickTrigger set to false after 2 seconds." << std::endl;
}

}  // namespace Utilities
