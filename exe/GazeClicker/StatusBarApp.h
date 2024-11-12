// StatusBarApp.h

#ifndef STATUSBARAPP_H
#define STATUSBARAPP_H

#include <opencv2/opencv.hpp>
#include <Cocoa/Cocoa.h>

// 상태바 애플리케이션을 생성하는 함수
void CreateStatusBarApp();

// 카메라 창을 생성하고 보여주는 함수
void CreateCameraWindow();

// 카메라 프레임을 업데이트하는 함수
void UpdateCameraFrame();

#endif // STATUSBARAPP_H
