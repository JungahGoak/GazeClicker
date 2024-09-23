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
		UI(std::vector<std::string> arguments);
		
		// 이미지 및 트랙바 설정
		void SetImage(const cv::Mat& canvas, float fx, float fy, float cx, float cy, int screen_width, int screen_height);
		void CreateTrackbars(); // 트랙바 생성 함수
		char ShowUI(); // 트랙바와 이미지 표시 및 종료

		// Gaze 관련 정보 설정
		void SetObservationGaze(const cv::Point3f& gaze_direction0, const cv::Point3f& gaze_direction1, const std::vector<cv::Point2f>& eye_landmarks2d, const std::vector<cv::Point3f>& eye_landmarks3d, double confidence);

		void SetFps(double fps);
		void SetScreenCoord(cv::Point2f rightScreenCoord, cv::Point2f leftScreenCoord, cv::Point2f screen_center);

		cv::Mat GetVisImage();

        bool vis_track;
        bool vis_hog;
        bool vis_align;
        bool vis_aus;

		// 트랙바 값
		int screen_face_distance;
		int scaling;

        // 확인 여부 플래그 확인
		bool IsConfirmed();

        // Can be adjusted to show less confident frames
		double visualisation_boundary = 0.4;

	private:
		cv::Mat captured_image; // 캡처된 이미지
		cv::Mat settings_page;
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
