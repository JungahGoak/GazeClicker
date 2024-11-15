#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <assert.h>
#include "GazeCoordinate.h"
#include "MappingScreen.h"

namespace GazeCoordinate {

GazeCoordinate::GazeCoordinate():
    hmm_models(GRID_SIZE * GRID_SIZE), 
    slopes(8, 0.1),  // 초기화
    intercept(50.0), 
    correction_rate(0.00001),
    isClickTrigger(false),
    isDwellTime(false) {
    setScreenSize();
}

void GazeCoordinate::updateSlope(cv::Point2f predictCoord, cv::Point2f clickCoord) {
	cv::Point2f screen_center = cv::Point2f(screen_width/2, screen_height/2);
    int direction = MappingScreen::calculateDirection(screen_center, clickCoord);  // 방향 계산
	double distance_from_center = cv::norm(clickCoord - screen_center);

    // 예측된 좌표와 실제 클릭 간의 오차 계산
    double error_x = std::abs(clickCoord.x - predictCoord.x);
    double error_y = std::abs(clickCoord.y - predictCoord.y);
    double error = error_x + error_y;

    // 동적 보정 비율 계산
    double dynamic_correction = correction_rate * error;

    // 예측 값이 화면 중심으로부터 멀리 있으면 기울기를 줄이고, 가까이 있으면 늘리는 로직
    if (cv::norm(clickCoord - screen_center) < cv::norm(predictCoord - screen_center)) {
        slopes[direction] = std::max(0.1, slopes[direction] + std::abs(dynamic_correction));
    } else {
        slopes[direction] = std::max(0.1, slopes[direction] - std::abs(dynamic_correction));
    }
    std::cout << "Updated slope for direction " << direction << ": " << slopes[direction] << std::endl;
	std::cout << "Current slopes: ";
    for (int i = 0; i < slopes.size(); ++i) {
        std::cout << "Direction " << i << ": " << slopes[i] << " ";
    }
    std::cout << std::endl;
}

void GazeCoordinate::updateSequence(cv::Point2f newCoord) {
    if (coord_sequence.size() >= COORD_SEQUENCE_LENGTH) {
        coord_sequence.pop_front();
    }
    coord_sequence.push_back(newCoord);
}

// isDwellTime의 getter (뮤텍스 보호)
bool GazeCoordinate::getIsDwellTime() {
    std::lock_guard<std::mutex> lock(dwellTimeMutex);  // 뮤텍스 잠금
    return isDwellTime;
}

// isDwellTime의 setter (뮤텍스 보호)
void GazeCoordinate::setIsDwellTime(bool value, std::chrono::steady_clock::time_point startTime) {
    std::lock_guard<std::mutex> lock(dwellTimeMutex);  // 뮤텍스 잠금
    isDwellTime = value;
    startedDwellTime = startTime;
}

// isClickTrigger의 getter (뮤텍스 보호)
bool GazeCoordinate::getIsClickTrigger() {
    std::lock_guard<std::mutex> lock(clickTriggerMutex);  // 뮤텍스 잠금
    return isClickTrigger;
}

// isClickTrigger의 setter (뮤텍스 보호)
void GazeCoordinate::setIsClickTrigger(bool value) {
    std::lock_guard<std::mutex> lock(clickTriggerMutex);  // 뮤텍스 잠금
    isClickTrigger = value;
}

// isClickTrigger의 getter (뮤텍스 보호)
bool GazeCoordinate::getIsPredictMode() {
    std::lock_guard<std::mutex> lock(isPredictModeMutex);  // 뮤텍스 잠금
    return isPredictMode;
}

// isClickTrigger의 setter (뮤텍스 보호)
void GazeCoordinate::setIsPredictMode(bool value) {
    std::lock_guard<std::mutex> lock(isPredictModeMutex);  // 뮤텍스 잠금
    isPredictMode = value;
}           

} // namespace GazeCoordinate