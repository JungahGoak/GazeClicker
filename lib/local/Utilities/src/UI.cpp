#include "UI.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace Utilities;

UI::UI(std::vector<std::string> arguments)
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

    // 드롭다운 목록이 열려 있을 때
    /*
    if (dropdown_open)
    {
        for (size_t i = 0; i < options.size(); ++i)
        {
            int option_y_start = dropdown_y_start + dropdown_height + i * (option_height + option_spacing);
            cv::rectangle(button_page, cv::Point(dropdown_x_start, option_y_start), 
                          cv::Point(dropdown_x_start + dropdown_width, option_y_start + option_height), 
                          cv::Scalar(70, 70, 70), -1);  // 옵션 배경
            cv::putText(button_page, options[i], 
                        cv::Point(dropdown_x_start + 10, option_y_start + option_height / 2 + 5), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);  // 옵션 텍스트
        }
    }
    */

    // 트랙바가 있는 창에 설정 페이지 표시
    cv::imshow("Settings", button_page);  // 버튼을 그린 페이지를 "Settings" 창에 표시

    char result = cv::waitKey(1);
    return result;
}

// 이미지 설정 함수
void UI::SetImage(const cv::Mat& canvas, float fx, float fy, float cx, float cy, int screen_width, int screen_height)
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

void UI::ShowCoord(cv::Point2f coord){
	cv::Scalar color = CV_RGB(0, 255, 0); 
	cv::circle(captured_image, coord, 5, color, 20);
}

char UI::ShowTrack(){

    bool ovservation_shown = false;
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

// 설정이 완료되었는지 확인하는 함수
bool UI::IsConfirmed()
{
    return this->confirmed;
}
