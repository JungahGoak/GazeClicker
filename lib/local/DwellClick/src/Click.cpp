#include "Click.h"
#include "UI.h"
#include "GazeCoordinate.h"
#include "GazeClickerConfig.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <thread>
#include <chrono>  // std::chrono 사용
#include <onnxruntime_cxx_api.h>  // GRU

#include <ApplicationServices/ApplicationServices.h>

namespace DwellClick {

// 응시 고정 여부 확인
bool Click::isFixation(const cv::Point2f& click_coord, const std::deque<cv::Point2f>& coord_sequence) {
    if (coord_sequence.empty()) return false;

    const auto& last_coord = coord_sequence.back();
    return cv::norm(click_coord - last_coord) <= fixation_distance_threshold;
}

// 응시 시간 측정 시작
void Click::startFixation(GazeCoordinate::GazeCoordinate& gazeCoord) {
    gazeCoord.setIsDwellTime(true, std::chrono::steady_clock::now());
}

// 응시 시간 업데이트 및 클릭 이벤트 처리
bool Click::updateDwellTime(const cv::Point2f& click_coord, GazeCoordinate::GazeCoordinate& gazeCoord) {
    if (isFixation(click_coord, gazeCoord.coord_sequence)) {
        if (!gazeCoord.getIsDwellTime()) {
            startFixation(gazeCoord);  // 응시 시간 측정 시작
        } else {
            // 응시 시간 확인
            auto now = std::chrono::steady_clock::now();
            double duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - gazeCoord.startedDwellTime).count() / 1000.0;

            if (duration >= fixation_threshold) {
                std::cout << "Fixation confirmed. Triggering click event." << std::endl;
                gazeCoord.click_coord = click_coord;
                gazeCoord.setIsDwellTime(false, std::chrono::steady_clock::time_point());

                // triggerClickEvent를 새로운 스레드에서 실행
                std::thread clickThread(&Click::triggerClickEvent, this, gazeCoord.click_coord, std::ref(gazeCoord));
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

void SimulateMouseClick(CGPoint point) {
    // 클릭 이벤트를 위한 CGEventRef 생성

    std::cout << "=======> Click Simulate: (" << point.x << " ," << point.y << ")" << std::endl;
    CGEventRef mouseDown = CGEventCreateMouseEvent(
        NULL, kCGEventLeftMouseDown, point, kCGMouseButtonLeft);
    CGEventRef mouseUp = CGEventCreateMouseEvent(
        NULL, kCGEventLeftMouseUp, point, kCGMouseButtonLeft);

    // 마우스 클릭 이벤트 발생
    CGEventPost(kCGHIDEventTap, mouseDown);
    CGEventPost(kCGHIDEventTap, mouseUp);

    // 메모리 해제
    CFRelease(mouseDown);
    CFRelease(mouseUp);
}

// 클릭 이벤트 발생 함수
void Click::triggerClickEvent(const cv::Point2f click_coord, GazeCoordinate::GazeCoordinate& gazeCoord) {

    std::cout << "Click event triggered!" << click_coord << std::endl;
    gazeCoord.setIsClickTrigger(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(fixation_threshold * 1000)));

    // 2초 후 isClickTrigger를 false로 설정
    gazeCoord.setIsClickTrigger(false);
    gazeCoord.setIsDwellTime(false, std::chrono::steady_clock::time_point());

    // 종료 시 yes, no인지 판단하고 클릭 이벤트 실행
    // 현재 x좌표가 클릭좌표보다 오른쪽이면 yes, 아니면 no
    if (click_coord.x > gazeCoord.coord_sequence.back().x){
        // click event 실행
        std::cout << "=====> Click YES" << std::endl;
        
        GazePattern::predict(gazeCoord.hmm_models, gazeCoord.coord_sequence);
        SimulateMouseClick(CGPointMake(click_coord.x, click_coord.y));
    } else {
        std::cout << "=====> Click NO" <<  "click 좌표: " << click_coord.x << " 예측 좌표: " << gazeCoord.coord_sequence.back().x << std::endl;
    }
}

}  // namespace Utilities
