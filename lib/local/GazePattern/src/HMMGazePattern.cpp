#include "GazePattern.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>

using namespace GazePattern;

int HMMGazePattern::getLabelFromCoord(cv::Point2f point) const {
    int labelRegionWidth = screen_width_ / label_grid_size_;
    int labelRegionHeight = screen_height_ / label_grid_size_;
    int regionX = point.x / labelRegionWidth;
    int regionY = point.y / labelRegionHeight;
    return regionY * label_grid_size_ + regionX;
}

void HMMGazePattern::initHMMGazePattern() {
    transitionCountMatrix.resize(label_grid_size_ * label_grid_size_);
    emissionMatrix.resize(label_grid_size_ * label_grid_size_, 0.0);
    observationCountMatrix.resize(label_grid_size_ * label_grid_size_, 0);
    totalObservationCount.clear();
    stateCount.clear();
}

int HMMGazePattern::predictHMM(std::deque<cv::Point2f> coordSequence, int seq_size) {
    if (coordSequence.size() != seq_size) {
        std::cerr << "Sequence size mismatch. Expected: " << seq_size << ", Received: " << coordSequence.size() << std::endl;
        return -1;
    }

    // 라벨에 대한 viterbi score 저장 (score 작을수록 가능성 높음)
    std::vector<double> viterbiScores(label_grid_size_ * label_grid_size_, std::numeric_limits<double>::max());
    // 최적 경로 추적을 위한 백포인터 배열
    std::vector<int> backPointers(label_grid_size_ * label_grid_size_, -1);

    cv::Point2f firstPoint = coordSequence.front();
    int firstLabel = getLabelFromCoord(firstPoint);

    viterbiScores[firstLabel] = 0.0;

    for (size_t i = 1; i < coordSequence.size(); ++i) {
        cv::Point2f currentPoint = coordSequence[i];
        int currentLabel = getLabelFromCoord(currentPoint);

        cv::Point2f prevPoint = coordSequence[i - 1];
        int prevLabel = getLabelFromCoord(prevPoint);

        double transitionProbability = MIN_PROB;
        if (transitionMatrix.size() > prevLabel && transitionMatrix[prevLabel].size() > currentLabel) {
            transitionProbability = transitionMatrix[prevLabel][currentLabel];
        }

        double emissionProbability = MIN_PROB;
        if (emissionMatrix.size() > currentLabel && totalObservationCount[currentLabel] > 0) {
            emissionProbability = static_cast<double>(observationCountMatrix[currentLabel]) / totalObservationCount[currentLabel];
        }

        double score = viterbiScores[prevLabel] - log(transitionProbability * emissionProbability + MIN_PROB);

        if (score < viterbiScores[currentLabel]) {
            viterbiScores[currentLabel] = score;
            backPointers[currentLabel] = prevLabel;
        }
    }

    cv::Point2f lastPoint = coordSequence.back();
    int lastLabel = getLabelFromCoord(lastPoint);

    int bestLabel = lastLabel;
    double bestScore = viterbiScores[lastLabel];

    for (size_t i = 0; i < viterbiScores.size(); ++i) {
        if (viterbiScores[i] < bestScore) {
            bestScore = viterbiScores[i];
            bestLabel = i;
        }
    }

    return bestLabel;
}

void HMMGazePattern::updateHMMParameters(const std::deque<cv::Point2f>& coordSequence, int seq_size, cv::Point2f clickedCoord) {
    if (coordSequence.size() != seq_size) {
        std::cerr << "Sequence size mismatch. Expected: " << seq_size << ", Received: " << coordSequence.size() << std::endl;
        return;
    }

    // 클릭된 좌표의 라벨을 계산
    int clickedLabel = getLabelFromCoord(clickedCoord);

    int prevLabel = -1;

    // coordSequence를 기반으로 전이 행렬을 업데이트
    for (size_t i = 0; i < coordSequence.size(); i += 3) {
        int currentLabel = getLabelFromCoord(coordSequence[i]);

        // 이전 라벨이 존재하면 전이 카운트 업데이트
        if (prevLabel != -1) {
            transitionCountMatrix[prevLabel][currentLabel]++;
            stateCount[prevLabel]++;
        }

        prevLabel = currentLabel;
    }

    // coordSequence 마지막 값과 클릭 좌표 전이 행렬 업데이트
    int lastLabel = getLabelFromCoord(coordSequence[-1]);
    transitionCountMatrix[lastLabel][clickedLabel]++;
    stateCount[lastLabel]++;

    // 전이 확률 계산
    for (size_t prev = 0; prev < transitionCountMatrix.size(); prev++) {
        if (stateCount[prev] > 0) {
            for (const auto& transition : transitionCountMatrix[prev]) {
                int nextLabel = transition.first;
                int transitionCount = transition.second;

                transitionMatrix[prev][nextLabel] = static_cast<double>(transitionCount) / stateCount[prev];
            }
        }
    }

    // 클릭된 좌표를 최종 라벨로 고려하여 관측 확률 업데이트
    observationCountMatrix[clickedLabel]++;
    totalObservationCount[clickedLabel]++;
    emissionMatrix[clickedLabel] = static_cast<double>(observationCountMatrix[clickedLabel]) / totalObservationCount[clickedLabel];
}
