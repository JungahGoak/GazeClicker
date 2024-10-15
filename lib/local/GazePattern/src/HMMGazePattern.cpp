#include "GazePattern.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>

using namespace GazePattern;

/* 방향 별 가중치 설정 */
// 방향 간의 거리를 계산하여 가중치 부여
/*
double HMMGazePattern::getWeight(int currentDir, int targetDir) {
    int distance = abs(currentDir - targetDir);
    
    // 방향 1과 8은 이어지는 관계이므로 이를 처리
    if (distance > 4) {
        distance = 8 - distance;
    }

    // 거리별 가중치 설정 (거리 0 = 자기 자신, 거리 1 = 가까운 방향, 거리 2 = 중간, 거리 3 이상 = 멀리)
    switch (distance) {
        case 0: return 1.0;  // 자기 자신 (가장 높은 확률)
        case 1: return 0.8;  // 가까운 방향
        case 2: return 0.5;  // 중간 거리
        default: return 0.2; // 멀리 떨어진 방향 (가장 낮은 확률)
    }
}
*/

/* label <-> coord */
int HMMGazePattern::getLabelFromCoord(cv::Point2f point) const {
    int labelRegionWidth = screen_width_ / label_grid_size_;
    int labelRegionHeight = screen_height_ / label_grid_size_;
    int regionX = point.x / labelRegionWidth;
    int regionY = point.y / labelRegionHeight;
    return regionY * label_grid_size_ + regionX;
}

cv::Point2f HMMGazePattern::getLabelCenter(int label){
    int labelRegionWidth = screen_width_ / label_grid_size_;
    int labelRegionHeight = screen_height_ / label_grid_size_;

    // label에서 x, y 좌표 구하기
    int labelX = label % label_grid_size_;  // 열 번호
    int labelY = label / label_grid_size_;  // 행 번호

    // 중심 좌표 계산
    float centerX = labelX * labelRegionWidth + labelRegionWidth / 2.0;
    float centerY = labelY * labelRegionHeight + labelRegionHeight / 2.0;

    return cv::Point2f(centerX, centerY);
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

    // 첫 번째 좌표 라벨의 viterbi 점수를 0으로 설정 (처음 시작 지점 확률 가장 높다고 가정)
    cv::Point2f firstPoint = coordSequence.front();
    int firstLabel = getLabelFromCoord(firstPoint);
    viterbiScores[firstLabel] = 0.0;

    // i+=3 각 좌표의 라벨 계산
    for (size_t i = 1; i < coordSequence.size(); i += 3) {
        cv::Point2f currentPoint = coordSequence[i];
        int currentLabel = getLabelFromCoord(currentPoint);

        cv::Point2f prevPoint = coordSequence[i - 1];
        int prevLabel = getLabelFromCoord(prevPoint);

        // 전이 확률 계산
        double transitionProbability = MIN_PROB;
        if (transitionMatrix.size() > prevLabel && transitionMatrix[prevLabel].size() > currentLabel) {
            transitionProbability = transitionMatrix[prevLabel][currentLabel];
        }

        // 방출 확률 계산
        double emissionProbability = MIN_PROB;
        if (emissionMatrix.size() > currentLabel && totalObservationCount[currentLabel] > 0) {
            emissionProbability = static_cast<double>(observationCountMatrix[currentLabel]) / totalObservationCount[currentLabel];
        }

        // viterbi score 점수 갱신
        double score = viterbiScores[prevLabel] - log(transitionProbability * emissionProbability + MIN_PROB);

        if (score < viterbiScores[currentLabel]) {
            viterbiScores[currentLabel] = score;
            backPointers[currentLabel] = prevLabel;
        }
    }

    // 최적의 라벨 찾음
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

int HMMGazePattern::predictHMM(std::deque<cv::Point2f> coordSequence, int seq_size) {
    if (coordSequence.size() < seq_size) {
        std::cerr << "Error: 시퀀스 크기가 충분하지 않습니다." << std::endl;
        return -1;
    }

    // 좌표 시퀀스를 1~8의 방향 시퀀스로 변환
    std::vector<int> directionSequence = convertCoordinatesToDirections(coordSequence);

    // Forward 확률을 계산하여 가장 가능성 높은 다음 상태를 예측
    std::vector<std::vector<double>> alpha(seq_size, std::vector<double>(NUM_STATES, 0.0));
    forward(directionSequence, transitionMatrix, emissionMatrix, alpha, seq_size);

    // 마지막 상태에서 가장 높은 확률을 가진 상태를 찾음
    int predictedLabel = -1;
    double maxProb = -std::numeric_limits<double>::infinity();
    for (int i = 0; i < NUM_STATES; ++i) {
        if (alpha[seq_size-1][i] > maxProb) {
            maxProb = alpha[seq_size-1][i];
            predictedLabel = i;
        }
    }

    if (predictedLabel == -1) {
        std::cerr << "Error: 예측할 수 없습니다. 모든 확률 값이 너무 낮습니다." << std::endl;
    }

    return predictedLabel;
}

void HMMGazePattern::printMatrix(){
    // 전이 행렬 출력
    std::cout << "===== HMM Transition Matrix =====" << std::endl;
    for (int i = 0; i < NUM_STATES; ++i) {
        for (int j = 0; j < NUM_STATES; ++j) {
            std::cout << transitionMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    // 관측 확률 행렬 출력
    std::cout << "===== HMM Emission Matrix =====" << std::endl;
    for (int i = 0; i < NUM_OBSERVATIONS; ++i) {
        std::cout << emissionMatrix[i] << " ";
    }
    std::cout << std::endl;
}

void HMMGazePattern::printVector(const std::vector<std::vector<double>>& vec) {
    for (const auto& row : vec) {
        for (const auto& elem : row) {
            std::cout << elem << " ";  // 각 원소를 출력
        }
        std::cout << std::endl;  // 한 행이 끝나면 개행
    }
}