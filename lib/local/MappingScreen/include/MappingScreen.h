#ifndef MAPPING_SCREEN_H
#define MAPPING_SCREEN_H

#include "LandmarkDetectorModel.h"

#include "opencv2/core/core.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui.hpp"

namespace MappingScreen
{
    float GetGazeDistance(cv::Vec2f gazeAngle, float screen_face_distance);

    cv::Point3f GetGazeCoord(cv::Point3f& gaze_point, cv::Point3f& gaze_vector, float gaze_distance);

    cv::Point2f GetScreenCoord(cv::Point3f GazeCoord, cv::Point3f EyeCoord, cv::Point2f ScreenCenterCoord, float scaling);

}
#endif // MAPPING_SCREEN_H