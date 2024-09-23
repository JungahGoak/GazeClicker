#include "UI.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace Utilities;

UI::UI(std::vector<std::string> arguments)
{
	// By default not visualizing anything
	this->vis_track = false;
	this->vis_hog = false;
	this->vis_align = false;
	this->vis_aus = false;

	for (size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-verbose") == 0)
		{
			this->vis_track = true;
			this->vis_align = true;
			this->vis_hog = true;
			this->vis_aus = true;
		}
		else if (arguments[i].compare("-vis-align") == 0)
		{
			this->vis_align = true;
		}
		else if (arguments[i].compare("-vis-hog") == 0)
		{
			this->vis_hog = true;
		}
		else if (arguments[i].compare("-vis-track") == 0)
		{
			this->vis_track = true;
		}
		else if (arguments[i].compare("-vis-aus") == 0)
		{
			this->vis_aus = true;
		}
	}

}

// 마우스 클릭 콜백 함수 (버튼을 직접 그려서 클릭 처리)
void UI::onMouse(int event, int x, int y, int, void* userdata)
{
    UI* ui = reinterpret_cast<UI*>(userdata);
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        // 확인 버튼 위치 확인 (예: 좌표 10, 260에서 150, 310까지)
        if (x >= 10 && x <= 190 && y >= 260 && y <= 310)
        {
            ui->confirmed = true;
            std::cout << "Confirmed settings - Distance: " << ui->screen_face_distance << ", Scaling: " << ui->scaling << std::endl;
        }
    }
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
void UI::CreateTrackbars()
{
    // 창 크기 및 초기 설정 페이지 생성
    cv::namedWindow("Settings", cv::WINDOW_AUTOSIZE);
    settings_page = cv::Mat::zeros(400, 300, CV_8UC3);  // 400x300 크기의 빈 화면

    // 트랙바 추가 (포인터 대신 콜백을 사용하여 값 변경 처리)
    cv::createTrackbar("Distance", "Settings", NULL, 100, onDistanceChange, this);
    cv::createTrackbar("Scaling", "Settings", NULL, 100, onScalingChange, this);

    // 마우스 콜백 설정 (확인 버튼 클릭 감지)
    cv::setMouseCallback("Settings", onMouse, this);
}

// 설정된 이미지와 트랙바 창을 표시하는 함수
char UI::ShowUI()
{
    // 설정 페이지에 버튼을 그리기
    settings_page.setTo(cv::Scalar(49, 52, 49));  // 배경 색을 설정

    // 버튼 외곽선 그리기 (흰색 테두리)
    cv::rectangle(settings_page, cv::Point(10, 260), cv::Point(190, 310), cv::Scalar(255, 255, 255), 2);  // 버튼 테두리
    cv::rectangle(settings_page, cv::Point(12, 262), cv::Point(188, 308), cv::Scalar(0, 255, 0), -1);  // 버튼 배경

    // 텍스트 중앙 배치
    int baseline = 0;
    cv::Size textSize = cv::getTextSize("Confirm", cv::FONT_HERSHEY_SIMPLEX, 1, 2, &baseline);
    cv::Point textOrg((190 - textSize.width) / 2, 290);  // 텍스트가 중앙에 오도록 설정

    // 버튼 텍스트 그리기
    cv::putText(settings_page, "Confirm", textOrg, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

    // 트랙바가 있는 창에 설정 페이지 표시
    cv::imshow("Settings", settings_page);  // 버튼을 그린 페이지를 "Settings" 창에 표시

    char result = cv::waitKey(1);
    return result;
}

// 이미지 설정 함수
void UI::SetImage(const cv::Mat& canvas, float fx, float fy, float cx, float cy, int screen_width, int screen_height)
{
    cv::resize(canvas.clone(), captured_image, cv::Size(screen_width, screen_height));
    cv::flip(captured_image, captured_image, 1);
}

// 설정이 완료되었는지 확인하는 함수
bool UI::IsConfirmed()
{
    return this->confirmed;
}
