#ifndef UI_H
#define UI_H

// System includes
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Utilities
{
	class UI {

	public:
		UI();
		
		// 이미지 및 트랙바 설정
		void SetImage(const cv::Mat& canvas, int screen_width, int screen_height);
		void CreateTrackbars(int distance, int scaling); // 트랙바 생성 함수
		char ShowUI(); // 트랙바와 이미지 표시 및 종료

		// Gaze 관련 정보 설정
		void SetObservationGaze(const cv::Point3f& gaze_direction0, const cv::Point3f& gaze_direction1, const std::vector<cv::Point2f>& eye_landmarks2d, const std::vector<cv::Point3f>& eye_landmarks3d, double confidence);

		void SetFps(double fps);
		void SetScreenCoord(cv::Point2f rightScreenCoord, cv::Point2f leftScreenCoord, cv::Point2f screen_center);
		void SetRedScreenCoord(cv::Point2f screen_center);
		void ShowCoord(cv::Point2f coord);
		void SetGrid(int screen_width, int screen_height, int grid_size);
		void SetPopup(cv::Point2f gazePoint, cv::Point2f popupCoord);

		cv::Mat GetVisImage();
		void drawCircleAsync(cv::Point2f center);
		char ShowTrack(int screen_width, int screen_height);

		// 트랙바 값
		int screen_face_distance = 50;
		int scaling = 30;

		// 버튼 값
		int button_x_start = 0;
		int button_y_start = 0;
		int button_width = 180;
		int button_height = 50;

        // 확인 여부 플래그 확인
		bool IsConfirmed();
		bool dropdown_opened = false;

        // Can be adjusted to show less confident frames
		double visualisation_boundary = 0.4;
		cv::Mat captured_image; // 캡처된 이미지

	private:
		
		cv::Mat button_page;
		cv::Mat button;
        bool confirmed = false; // 사용자 설정 확인 여부


		static void onDistanceChange(int pos, void* userdata);  // Distance 트랙바 콜백
		static void onScalingChange(int pos, void* userdata);   // Scaling 트랙바 콜백

		static void onConfirm(int, void*); // 확인 버튼 콜백
		static void onExit(int, void*); // 종료 버튼 콜백
        
		// 마우스 클릭으로 버튼 구현
		static void onMouse(int event, int x, int y, int flags, void* userdata);  // 마우스 클릭 콜백 (확인 버튼 처리)
	};
}

#endif
