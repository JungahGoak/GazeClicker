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
bool Click::isFixation(const cv::Point2f& cur_coord, const std::deque<cv::Point2f>& coord_sequence) {
    if (coord_sequence.empty()) return false;

    const auto& last_coord = coord_sequence.back();
    return cv::norm(cur_coord - last_coord) <= fixation_distance_threshold;
}

// 응시 시간 측정 시작
void Click::startFixation(GazeCoordinate::GazeCoordinate& gazeCoord) {
    gazeCoord.setIsDwellTime(true, std::chrono::steady_clock::now());
}

// 응시 시간 업데이트 및 클릭 이벤트 처리
bool Click::updateDwellTime(const cv::Point2f& cur_coord, GazeCoordinate::GazeCoordinate& gazeCoord) {
    if (isFixation(cur_coord, gazeCoord.coord_sequence)) {
        if (!gazeCoord.getIsDwellTime()) {
            startFixation(gazeCoord);  // 응시 시간 측정 시작
        } else {
            // 응시 시간 확인
            auto now = std::chrono::steady_clock::now();
            double duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - gazeCoord.startedDwellTime).count() / 1000.0;

            std::cout << "Fixation duration: " << duration << " seconds" << std::endl;

            if (duration >= fixation_threshold) {
                std::cout << "Fixation confirmed. Triggering click event." << std::endl;
                gazeCoord.click_coord = cur_coord;
                gazeCoord.setIsDwellTime(false, std::chrono::steady_clock::time_point());

                // triggerClickEvent를 새로운 스레드에서 실행
                std::thread clickThread(&Click::triggerClickEvent, this, cur_coord, std::ref(gazeCoord));
                clickThread.detach();

                return true;
            }
        }
    } else {
        // 고정 해제 시 타이머 초기화
        gazeCoord.setIsDwellTime(false, std::chrono::steady_clock::time_point());
    }
    return false;
}

// 클릭 이벤트 발생 함수
void Click::triggerClickEvent(const cv::Point2f cur_coord, GazeCoordinate::GazeCoordinate& gazeCoord) {

    std::cout << "Click event triggered!" << cur_coord << std::endl;
    gazeCoord.setIsClickTrigger(true);
    std::this_thread::sleep_for(std::chrono::seconds(fixation_threshold));

    // 2초 후 isClickTrigger를 false로 설정
    gazeCoord.setIsClickTrigger(false);
    gazeCoord.setIsDwellTime(false, std::chrono::steady_clock::time_point());

    // 종료 시 yes, no인지 판단하고 클릭 이벤트 실행
    // 현재 x좌표가 클릭좌표보다 오른쪽이면 yes, 아니면 no
    if (cur_coord.x > gazeCoord.click_coord.x){
        // click event 실행
        std::cout << "YYYYYEEEEESSSSSS" << std::endl;
    } else {
        std::cout << "NNNNOOOOOOOOOOOO" << std::endl;
    }
}

}  // namespace Utilities
