#ifndef GAZE_PATTERN_H
#define GAZE_PATTERN_H

#include <vector>
#include <unordered_map>
#include <map>
#include <deque>
#include <opencv2/core.hpp>

namespace GazePattern
{
    class HMMGazePattern {
    public:
        HMMGazePattern(int screen_width, int screen_height, int label_grid_size = 10){
            screen_width_ = screen_width;
            screen_height_ = screen_height;
            label_grid_size_ = label_grid_size;
            initHMMGazePattern();
        }

        // HMM 초기화 함수
        void initHMMGazePattern();

        // HMM 기반 예측 함수
        int predictHMM(std::deque<cv::Point2f> coordSequence, int seq_size);

        // HMM 파라미터 업데이트
        void HMMGazePattern::updateHMMParameters(const std::deque<cv::Point2f>& coordSequence, int seq_size, cv::Point2f clickedCoord);


    private:
        int screen_width_;
        int screen_height_;
        int label_grid_size_;

        std::vector<std::unordered_map<int, int>> transitionCountMatrix;
        std::unordered_map<int, std::map<int, double>> transitionMatrix;
        std::vector<double> emissionMatrix;
        std::vector<int> observationCountMatrix;
        std::unordered_map<int, int> stateCount;
        std::unordered_map<int, int> totalObservationCount;

        const double MIN_PROB = 1e-6;

        // 좌표를 기반으로 라벨을 계산하는 함수 (private 함수)
        int getLabelFromCoord(cv::Point2f point) const;
    };
}

#endif // GAZE_PATTERN_H
