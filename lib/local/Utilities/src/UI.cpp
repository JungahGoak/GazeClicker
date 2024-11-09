#include "UI.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <thread> 
#include <chrono>

using namespace Utilities;

UI::UI()
    //down_open(false)  // Default option and dropdown state
{
    //options = {"c", "l", "option1", "option2"};  // 드롭다운 메뉴 옵션 설정
}

// 마우스 클릭 콜백 함수 (버튼을 직접 그려서 클릭 처리)
void UI::onMouse(int event, int x, int y, int, void* userdata)
{
    UI* ui = reinterpret_cast<UI*>(userdata);
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        // 확인 버튼 위치 확인 (예: 좌표 10, 260에서 150, 310까지)
        if (x >= ui->button_x_start && x <= (ui->button_x_start + ui->button_width) &&
            y >= ui->button_y_start && y <= (ui->button_y_start + ui->button_height))
        {
            ui->confirmed = true;
            std::cout << "Confirmed settings - Distance: " << ui->screen_face_distance << ", Scaling: " << ui->scaling << std::endl;
        }
    }

    // 드롭다운 목록이 열려 있을 때, 옵션 클릭 처리
    /*
    if (ui->dropdown_open)
    {
        for (size_t i = 0; i < ui->options.size(); ++i)
        {
            int option_y_start = ui->dropdown_y_start + ui->dropdown_height + i * (ui->option_height + ui->option_spacing);
            if (x >= ui->dropdown_x_start && x <= (ui->dropdown_x_start + ui->dropdown_width) &&
                y >= option_y_start && y <= (option_y_start + ui->option_height))
            {
                ui->selected_option = ui->options[i];
                ui->dropdown_open = false;  // 선택 후 드롭다운 닫기
                std::cout << "Selected Click Action: " << ui->selected_option << std::endl;
                break;
            }
        }
    }
    */

}

// 트랙바 값 변경 콜백 함수 (Distance)
void UI::onDistanceChange(int pos, void* userdata)
{
    UI* ui = reinterpret_cast<UI*>(userdata);
    ui->screen_face_distance = pos;
}

// 트랙바 값 변경 콜백 함수 (Scaling)
void UI::onScalingChange(int pos, void* userdata)
{
    UI* ui = reinterpret_cast<UI*>(userdata);
    ui->scaling = pos;
}

// 트랙바와 버튼을 설정하는 함수
void UI::CreateTrackbars(int distance, int scaling)
{
    // 창 크기 및 초기 설정 페이지 생성

    cv::namedWindow("Settings", cv::WINDOW_AUTOSIZE);
    cv::resizeWindow("Settings", 1500, 300);

    // button_page를 창 크기에 맞게 설정
    button_page = cv::Mat::zeros(button_height, button_width, CV_8UC3);  // 창 크기와 동일한 크기의 빈 화면


    // 트랙바 추가 (포인터 대신 콜백을 사용하여 값 변경 처리)
    cv::createTrackbar("Distance (cm): ", "Settings", &distance, 100, onDistanceChange, this);
    cv::createTrackbar("Scaling: ", "Settings", &scaling, 100, onScalingChange, this);

    // 마우스 콜백 설정 (확인 버튼 클릭 감지)
    cv::setMouseCallback("Settings", onMouse, this);
}

// 설정된 이미지와 트랙바 창을 표시하는 함수
char UI::ShowUI()
{
    // 설정 페이지에 버튼을 그리기
    button_page.setTo(cv::Scalar(43, 42, 40));  // 배경 색을 설정

    // 창 크기를 가져옴 (button_page는 창을 표시할 Mat 객체)
    int window_width = button_page.cols;
    int window_height = button_page.rows;

    // 버튼 외곽선 그리기 (흰색 테두리)
    cv::rectangle(button_page, cv::Point(button_x_start - 2, button_y_start - 2), cv::Point(button_x_start + button_width + 2, button_y_start + button_height + 2), cv::Scalar(255, 255, 255), 2);  // 버튼 테두리
    cv::rectangle(button_page, cv::Point(button_x_start, button_y_start), cv::Point(button_x_start + button_width, button_y_start + button_height), cv::Scalar(50, 50, 50), -1);  // 버튼 배경

    // 텍스트 중앙 배치
    int baseline = 0;
    double fontScale = 0.5;
    cv::Size textSize = cv::getTextSize("Setting", cv::FONT_HERSHEY_SIMPLEX, fontScale, 2, &baseline);
    cv::Point textOrg(button_x_start + (button_width - textSize.width) / 2, button_y_start + (button_height + textSize.height) / 2 - baseline);

    // 버튼 텍스트 그리기 (그림 위치, 텍스트 내용, 시작 위치, 폰트 종류, 크기, 색깔, 두께)
    cv::putText(button_page, "Setting", textOrg, cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(255, 255, 255), 1);

    // 트랙바가 있는 창에 설정 페이지 표시
    cv::imshow("Settings", button_page);  // 버튼을 그린 페이지를 "Settings" 창에 표시

    char result = cv::waitKey(1);
    return result;
}

// 이미지 설정 함수
void UI::SetImage(const cv::Mat& canvas, int screen_width, int screen_height)
{
    cv::resize(canvas.clone(), captured_image, cv::Size(screen_width, screen_height));
    cv::flip(captured_image, captured_image, 1);
}

void UI::SetScreenCoord(cv::Point2f rightScreenCoord, cv::Point2f leftScreenCoord, cv::Point2f screen_center){

	cv::Scalar red = CV_RGB(255, 0, 0); // 빨간색
    cv::Scalar blue = CV_RGB(0, 0, 255); // 파란색

	cv::circle(captured_image, rightScreenCoord, 5, blue, 20);
	cv::circle(captured_image, leftScreenCoord, 5, blue, 20);
    cv::circle(captured_image, screen_center, 5, red, 10);

}

void UI::SetRedScreenCoord(cv::Point2f screen_center){

	cv::Scalar red = CV_RGB(255, 0, 0); // 빨간색

    cv::circle(captured_image, screen_center, 5, red, 20);

}

void UI::SetPopup(cv::Point2f cur_coord, cv::Point2f popup_coord){
    
    // popup_coord 위치에 버튼 설정
    int buttonWidth = 110, buttonHeight = 60;

    cv::Rect yesButton(static_cast<int>(popup_coord.x) - buttonWidth - 10,  // 왼쪽에 YES
                       static_cast<int>(popup_coord.y) - buttonHeight / 2,  // 세로 중앙 정렬
                       buttonWidth, buttonHeight);

    cv::Rect noButton(static_cast<int>(popup_coord.x) + 10,  // 오른쪽에 NO
                      static_cast<int>(popup_coord.y) - buttonHeight / 2, 
                      buttonWidth, buttonHeight);

    bool isYesSelected = false, isNoSelected = false;

    // 기본 버튼 색상 설정
    cv::Scalar yesColor(102, 178, 255);  // 파란색 (YES)
    cv::Scalar noColor(102, 178, 255);   // 파란색 (NO)

    int midX = (yesButton.x + yesButton.width + noButton.x) / 2;

    // YES 버튼 안에 있으면 빨간색으로 표시
    if (cur_coord.x < midX) {
        yesColor = cv::Scalar(0, 0, 255);  // 빨간색
        isYesSelected = true;
        isNoSelected = false;
        std::cout << "Gaze on YES button" << std::endl;
    }
    else {
        noColor = cv::Scalar(0, 0, 255);  // 빨간색
        isNoSelected = true;
        isYesSelected = false;
        std::cout << "Gaze on NO button" << std::endl;
    }

    // YES 버튼 그리기
    cv::rectangle(captured_image, yesButton, yesColor, -1);
    cv::putText(captured_image, "YES", cv::Point(yesButton.x + 15, yesButton.y + 40),
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    // NO 버튼 그리기
    cv::rectangle(captured_image, noButton, noColor, -1);
    cv::putText(captured_image, "NO", cv::Point(noButton.x + 15, noButton.y + 40),
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

}

void UI::ShowCoord(cv::Point2f coord){
	cv::Scalar color = CV_RGB(0, 255, 0); 
	cv::circle(captured_image, coord, 20, color, 20);
}

char UI::ShowTrack(int screen_width, int screen_height){

    bool ovservation_shown = false;

    // 정확한 크기의 창 생성
    cv::namedWindow("tracking result", cv::WINDOW_NORMAL);  // 창 크기 조절 가능
    cv::resizeWindow("tracking result", screen_width, screen_height);  // 창 크기 설정

    cv::imshow("tracking result", captured_image);
    ovservation_shown = true;

    // Only perform waitKey if something was shown
	char result = '\0';
	if (ovservation_shown)
	{
		result = cv::waitKey(1);
	}
	return result;
}


void UI::SetGrid(int screen_width, int screen_height, int grid_size)
{
    int labelRegionWidth = screen_width / grid_size;
    int labelRegionHeight = screen_height / grid_size;

    // 격자와 라벨 그리기
    for (int y = 0; y < grid_size; ++y) {
        for (int x = 0; x < grid_size; ++x) {
            // 현재 격자의 왼쪽 위 좌표
            int top_left_x = x * labelRegionWidth;
            int top_left_y = y * labelRegionHeight;

            // 현재 격자의 오른쪽 아래 좌표
            int bottom_right_x = (x + 1) * labelRegionWidth;
            int bottom_right_y = (y + 1) * labelRegionHeight;

            // 격자 그리기 (흰색)
            cv::rectangle(captured_image, 
                          cv::Point(top_left_x, top_left_y), 
                          cv::Point(bottom_right_x, bottom_right_y), 
                          cv::Scalar(255, 255, 255), 1);

            // 라벨 텍스트 생성
            int label = y * grid_size + x;
            std::string labelText = std::to_string(label);

            // 라벨을 격자 중앙에 출력 (작은 글씨, 흰색)
            int text_x = top_left_x + labelRegionWidth / 4;
            int text_y = top_left_y + labelRegionHeight / 2;

            cv::putText(captured_image, labelText, 
                        cv::Point(text_x, text_y), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                        cv::Scalar(255, 255, 255), 1);
        }
    }
}

// 설정이 완료되었는지 확인하는 함수
bool UI::IsConfirmed()
{
    return this->confirmed;
}
