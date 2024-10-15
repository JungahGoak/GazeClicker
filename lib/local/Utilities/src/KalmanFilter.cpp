#include "stdafx_ut.h"

#include "KalmanFilter.h"

using namespace Utilities;

// kalman filter
void KalmanFilter::initKalmanfilter() {

    int stateSize = 4;   // [x, y, vx, vy]
    int measSize = 2;    // [x, y]
    int contrSize = 0;   // 제어 입력 없음

    kf.init(stateSize, measSize, contrSize, CV_32F);

    // 상태 전이 행렬 A (Δt = 1 가정)
    kf.transitionMatrix = (cv::Mat_<float>(4, 4) << 
                           1, 0, 1, 0, 
                           0, 1, 0, 1, 
                           0, 0, 1, 0, 
                           0, 0, 0, 1);

    // 제어 입력 행렬 B
    kf.controlMatrix = cv::Mat::zeros(4, 2, CV_32F); 

    // 측정 모델 행렬 H
    kf.measurementMatrix = (cv::Mat_<float>(2, 4) << 
                            1, 0, 0, 0, 
                            0, 1, 0, 0);

    // 프로세스 노이즈 공분산 행렬 Q
    cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-2));

    // 측정 노이즈 공분산 행렬 R
    cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-1));

    // 상태 공분산 행렬 P (초기 불확실성)
    cv::setIdentity(kf.errorCovPost, cv::Scalar::all(1));

    // 상태 및 측정 벡터 초기화
    state = cv::Mat::zeros(stateSize, 1, CV_32F);
    measurement = cv::Mat::zeros(measSize, 1, CV_32F);

}

// 예측 단계
cv::Point2f KalmanFilter::predict() {
    cv::Mat prediction = kf.predict();
    cv::Point2f predictPt(prediction.at<float>(0), prediction.at<float>(1)); // [x, y]
    return predictPt;
}

// 측정값을 통해 보정
void KalmanFilter::correct(cv::Point2f coord) {
    measurement.at<float>(0) = coord.x;
    measurement.at<float>(1) = coord.y;
    kf.correct(measurement);
}

// 현재 보정된 위치 반환
cv::Point2f KalmanFilter::getCorrectedPosition() {
    return cv::Point2f(kf.statePost.at<float>(0), kf.statePost.at<float>(1)); // [x, y]
}
