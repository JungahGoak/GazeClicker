#ifndef GAZE_CLICKER_CONFIG_H
#define GAZE_CLICKER_CONFIG_H

#include <deque>
#include <opencv2/core.hpp>
#include "HMM.h"


// 화면 및 HMM 설정
constexpr int GRID_SIZE = 10;          // 화면을 나누는 그리드 크기
constexpr int NUM_STATES = 10;         // HMM의 상태 수
constexpr int NUM_OBSERVATIONS = 8;    // HMM의 관측 기호 수
constexpr int COORD_SEQUENCE_LENGTH = 20;    // 좌표 시퀀스의 길이
constexpr int PREDICT_SEQUENCE_LENGTH = 10; // 예측 시퀀스 길이

// 화면 크기 변수 (정의는 cpp 파일에서)
extern int screen_width;
extern int screen_height;

void setScreenSize();

// 좌표 시퀀스를 저장하는 덱
#endif // GAZE_CLICKER_CONFIG_H