#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>

#include "MappingScreen.h"

#include "LandmarkDetectorUtils.h"
#include "LandmarkDetectorFunc.h"
#include "RotationHelpers.h"

using namespace MappingScreen;

float MappingScreen::GetGazeDistance(cv::Vec2f gazeAngle, float screen_face_distance)
{
	//gazeAngle[0] = gazeAngleX (좌우) 우: 양수 / 좌: 음수
	//gazeAngle[1] = gazeAngleY (상하) 하: 양수 /  상: 음수
	return sqrt(pow(screen_face_distance / cos(gazeAngle[0]), 2) + pow(screen_face_distance * tan(gazeAngle[1]), 2));
}

cv::Point3f MappingScreen::GetGazeCoord(cv::Point3f& gaze_point, cv::Point3f& gaze_vector, float gaze_distance)
{
    return gaze_point + gaze_vector * gaze_distance;
}

cv::Point2f MappingScreen::GetScreenCoord(cv::Point3f GazeCoord, cv::Point3f EyeCoord, cv::Point2f ScreenCenterCoord, float scaling){
    
    return cv::Point2f(-(GazeCoord.x-EyeCoord.x) * scaling + ScreenCenterCoord.x,
                        (GazeCoord.y-EyeCoord.y) * scaling + ScreenCenterCoord.y);
}