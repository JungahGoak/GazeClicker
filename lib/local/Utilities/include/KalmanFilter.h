#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <opencv2/core.hpp>
#include <opencv2/video/tracking.hpp> 

namespace Utilities {

    class KalmanFilter {
        public:
            cv::KalmanFilter kf;
            cv::Mat state;    // [x, y, vx, vy] 상태 벡터
            cv::Mat measurement; // [x, y] 측정 벡터

            KalmanFilter() {
                initKalmanfilter();
            }

            void initKalmanfilter();
            cv::Point2f predict();
            void correct(cv::Point2f coord);
            cv::Point2f getCorrectedPosition();
    };
}

#endif // KALMAN_FILTER_H