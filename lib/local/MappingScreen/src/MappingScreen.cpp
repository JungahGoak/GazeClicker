#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>

#include "MappingScreen.h"
#include "GazeClickerConfig.h"

#include "LandmarkDetectorUtils.h"
#include "LandmarkDetectorFunc.h"
#include "RotationHelpers.h"

namespace MappingScreen {

float GetGazeDistance(cv::Vec2f gazeAngle, float screen_face_distance)
{
	//gazeAngle[0] = gazeAngleX (좌우) 우: 양수 / 좌: 음수
	//gazeAngle[1] = gazeAngleY (상하) 하: 양수 /  상: 음수
	return sqrt(pow(screen_face_distance / cos(gazeAngle[0]), 2) + pow(screen_face_distance * tan(gazeAngle[1]), 2));
}

cv::Point3f GetGazeCoord(cv::Point3f& gaze_point, cv::Point3f& gaze_vector, float gaze_distance)
{
    return gaze_point + gaze_vector * gaze_distance;
}

cv::Point2f GetScreenCoord(cv::Point3f GazeCoord, cv::Point3f EyeCoord, cv::Point2f ScreenCenterCoord, float scaling){
    
    return cv::Point2f(-(GazeCoord.x-EyeCoord.x) * scaling + ScreenCenterCoord.x,
                        (GazeCoord.y-EyeCoord.y) * scaling + ScreenCenterCoord.y);
}

int getDirectionFromAngle(float angle) {
        if (angle < 0) {
            angle += 360;
        }

        // 각도에 따라 1~8 방향으로 변환
        if (angle >= 0 && angle < 45) return 0;
        if (angle >= 45 && angle < 90) return 1;
        if (angle >= 90 && angle < 135) return 2;
        if (angle >= 135 && angle < 180) return 3;
        if (angle >= 180 && angle < 225) return 4;
        if (angle >= 225 && angle < 270) return 5;
        if (angle >= 270 && angle < 315) return 6;
        return 7;
}

int calculateDirection(const cv::Point2f& startPoint, const cv::Point2f& endPoint) {
    float deltaX = endPoint.x - startPoint.x;
    float deltaY = endPoint.y - startPoint.y;

    float angle = atan2(deltaY, deltaX) * 180.0 / CV_PI;
    return getDirectionFromAngle(angle);
}

std::vector<int> convertCoordinatesToDirections(const std::deque<cv::Point2f>& coordSequence) {
    std::vector<int> directionSequence;

    for (size_t i = 0; i < coordSequence.size() - 1; ++i) {
        int direction = calculateDirection(coordSequence[i], coordSequence[i + 1]);
        directionSequence.push_back(direction);
    }
    return directionSequence;
}

int getLabelFromCoord(const cv::Point2f& point) {
    int labelRegionWidth = screen_width / GRID_SIZE;
    int labelRegionHeight = screen_height / GRID_SIZE;

    int regionX = point.x / labelRegionWidth;
    int regionY = point.y / labelRegionHeight;

    return regionY * GRID_SIZE + regionX;
}

cv::Point2f getLabelCenter(int label) {
    int labelRegionWidth = screen_width / GRID_SIZE;
    int labelRegionHeight = screen_height / GRID_SIZE;

    int regionX = label % GRID_SIZE;
    int regionY = label / GRID_SIZE;

    float centerX = (regionX + 0.5) * labelRegionWidth;
    float centerY = (regionY + 0.5) * labelRegionHeight;

    return cv::Point2f(centerX, centerY);
}

std::vector<int> getNeighboringLabels(int label) {
    std::vector<int> neighbors;
    int x = label % GRID_SIZE;   // 라벨의 X 좌표
    int y = label / GRID_SIZE;   // 라벨의 Y 좌표

    // 상하좌우 및 대각선 위치 탐색
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {

            int nx = x + dx;
            int ny = y + dy;
            
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                neighbors.push_back(ny * GRID_SIZE + nx);
            }
        }
    }

    return neighbors;
}

}