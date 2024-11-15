// GazeClicker.cpp : 

// Local includes

// OpenCV 헤더 포함을 Objective-C 헤더 포함보다 먼저 수행
#include <opencv2/opencv.hpp>

#include <ApplicationServices/ApplicationServices.h> // Quartz Event Services를 위한 헤더

#include "LandmarkCoreIncludes.h"
#include "GazeClickerConfig.h"

#include <GazePattern.h>
#include <Face_utils.h>
#include <FaceAnalyser.h>
#include <GazeEstimation.h>
#include <SequenceCapture.h>
#include <Visualizer.h>
#include <MappingScreen.h>
#include <GazePattern.h>
#include <VisualizationUtils.h>
#include <KalmanFilter.h>
#include <UI.h>
#include "Click.h"
#include "GazeCoordinate.h"

#include <algorithm>

#include "StatusBarApp.h"

#ifndef CONFIG_DIR
#define CONFIG_DIR "~"
#endif

#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort(const std::string & error)
{
	std::cout << error << std::endl;
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

std::vector<std::string> get_arguments(int argc, char **argv)
{

	std::vector<std::string> arguments;

	for (int i = 0; i < argc; ++i)
	{
		arguments.push_back(std::string(argv[i]));
	}
	return arguments;
}

GazeCoordinate::GazeCoordinate gazeCoord;

// 마우스 이벤트 콜백 함수
CGEventRef mouseCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *userInfo) {
    if (type == kCGEventLeftMouseDown) {

		// 클릭 좌표
        CGPoint mouseLocation = CGEventGetLocation(event);
		
        // 좌표 시퀀스의 크기가 맞으면 HMM 파라미터 업데이트
        if (gazeCoord.coord_sequence.size() == COORD_SEQUENCE_LENGTH) {

            // 좌표 시퀀스를 관측 심볼(1~8)로 변환
            std::vector<int> directionSequence = MappingScreen::convertCoordinatesToDirections(gazeCoord.coord_sequence);
			
			// 1~8 변환된 방향 시퀀스 출력
			std::cout << "==> 좌표 시퀀스 변환(1~8): ";
			for (const auto& direction : directionSequence) {
				std::cout << direction << " ";
			}
			std::cout << std::endl;

			// 클릭 좌표 출력
			std::cout << "===> 마우스 클릭 좌표: (" << mouseLocation.x << ", " << mouseLocation.y << ")" << std::endl;

            // 클릭된 좌표를 라벨 값으로 변환
            int clickedLabel = MappingScreen::getLabelFromCoord(cv::Point2f(mouseLocation.x, mouseLocation.y));
			std::cout << "====> 클릭 좌표 변환(label): " << clickedLabel << " screen size: " << screen_width << ' ' << screen_height <<std::endl;

			// 최근 좌표와 클릭된 좌표 비교 후 기울기 업데이트
			if (!gazeCoord.coord_sequence.empty()) {
				gazeCoord.updateSlope(gazeCoord.coord_sequence.back(), cv::Point2f(mouseLocation.x, mouseLocation.y));  // 기울기 업데이트
			}

            // 클릭된 라벨에 해당하는 HMM 모델 유무 확인
            if (gazeCoord.hmm_models[clickedLabel] == nullptr) {
                // 해당 라벨에 대한 HMM 모델이 없으면 새로 생성
                gazeCoord.hmm_models[clickedLabel] = std::make_unique<GazePattern::HMM>(NUM_STATES, NUM_OBSERVATIONS);
                std::cout << "HMM created for label: " << clickedLabel << std::endl;
            }

            // 해당 라벨의 HMM 모델 업데이트
            GazePattern::HMM& selectedHMM = *gazeCoord.hmm_models[clickedLabel];
            selectedHMM.baum_welch(directionSequence, 1);
            //selectedHMM.print_matrices();

            std::cout << "HMM updated with sequence and clicked label: " << clickedLabel << std::endl;
        } else {
            std::cerr << "Sequence size mismatch. Current sequence size: " << gazeCoord.coord_sequence.size() << ", Expected: " << COORD_SEQUENCE_LENGTH << std::endl;
        }
    }
    return event;  // 다른 이벤트 처리로 전달
}

// 마우스 이벤트 처리 루프를 실행하는 함수
void startMouseEventLoop() {

	CFMachPortRef eventTap = CGEventTapCreate(
		kCGHIDEventTap,                      // 모든 HID(마우스/키보드) 이벤트 감지
		kCGHeadInsertEventTap,               // 이벤트를 맨 앞에 삽입
		kCGEventTapOptionDefault,            // 기본 옵션
		CGEventMaskBit(kCGEventLeftMouseDown), // 왼쪽 마우스 버튼 클릭 이벤트 감지
		mouseCallback,                       // 이벤트 처리 콜백 함수
		nullptr
		);

    if (!eventTap) {
        std::cerr << "Failed to create event tap. Make sure to enable accessibility permissions." << std::endl;
        return;
    }

    // 이벤트 루프에 탭 추가
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    std::cout << "Event tap created successfully!" << std::endl;

    // 이벤트 루프 실행 (스레드에서 실행될 부분)
    CFRunLoopRun();
}

// screen_coord가 화면 크기를 벗어나지 않도록 범위 제한
cv::Point2f clampToScreen(const cv::Point2f& coord) {
    float clamped_x = std::clamp(coord.x, 0.0f, static_cast<float>(screen_width - 1));
    float clamped_y = std::clamp(coord.y, 0.0f, static_cast<float>(screen_height - 1));
    return cv::Point2f(clamped_x, clamped_y);
}

int runGazeClickerTasks(int argc, char **argv){

	INFO_STREAM( "runGazeClickerTasks" );

	Utilities::UI& ui = gazeCoord.ui;
	DwellClick::Click clickManager;

	std::vector<std::string> arguments = get_arguments(argc, argv);

	/*
	// UI 생성
	ui.CreateTrackbars(50, 70);
	ui.ShowUI();

	// 트랙바의 "확인" 버튼이 눌릴 때까지 대기
    while (!ui.IsConfirmed()) {
        cv::waitKey(100);  // 100ms 간격으로 확인 여부 체크
    }
	

	// User 입력 변수
	float scaling = ui.scaling;
	double screen_face_distance = ui.screen_face_distance;
	*/
	float scaling = 70;
	double screen_face_distance = 50;



	// no arguments: output usage
	if (arguments.size() == 1)
	{
		std::cout << "For command line arguments see:" << std::endl;
		std::cout << " https://github.com/TadasBaltrusaitis/OpenFace/wiki/Command-line-arguments";
		return 0;
	}

	// Load the modules that are being used for tracking and face analysis
	// Load face landmark detector
	LandmarkDetector::FaceModelParameters det_parameters(arguments);
	// Always track gaze in feature extraction
	LandmarkDetector::CLNF face_model(det_parameters.model_location);

	if (!face_model.loaded_successfully)
	{
		std::cout << "ERROR: Could not load the landmark detector" << std::endl;
		return 1;
	}

    	if (!face_model.eye_model)
	{
		std::cout << "WARNING: no eye model found" << std::endl;
	}

    Utilities::SequenceCapture sequence_reader;

	// A utility for visualizing the results
	Utilities::Visualizer visualizer(arguments);

	// Tracking FPS for visualization
	Utilities::FpsTracker fps_tracker;
	fps_tracker.AddFrame();

	// kalman filter
	Utilities::KalmanFilter kf;

	// 마우스 이벤트 루프를 별도의 스레드에서 실행
	// bind: std::bind는 함수 호출 시 필요한 인자를 고정시켜 std::thread에 전달할 수 있도록 해줍니다. 이 경우, startMouseEventLoop 함수에 gazePattern 객체 포인터를 전달
    std::thread mouseEventThread(std::bind(startMouseEventLoop), nullptr);
	// 스레드를 detach하여 메인 스레드가 마우스 이벤트를 기다리지 않게 함
    mouseEventThread.detach();
	
	int predictedRegion;

    while (true) // this is not a for loop as we might also be reading from a webcam
	{

        // The sequence reader chooses what to open based on command line arguments provided
		if (!sequence_reader.Open(arguments))
			break;

		INFO_STREAM("Device or file opened");

		if (sequence_reader.IsWebcam())
		{
			INFO_STREAM("WARNING: using a webcam in feature extraction, Action Unit predictions will not be as accurate in real-time webcam mode");
			INFO_STREAM("WARNING: using a webcam in feature extraction, forcing visualization of tracking to allow quitting the application (press q)");
			visualizer.vis_track = true;
		}

        cv::Mat captured_image;
		captured_image = sequence_reader.GetNextFrame();	

		// For reporting progress
		double reported_completion = 0;

        INFO_STREAM("Starting tracking");
		while (!captured_image.empty())
		{
            // Converting to grayscale
			cv::Mat_<uchar> grayscale_image = sequence_reader.GetGrayFrame();

            // The actual facial landmark detection / tracking
			bool detection_success = LandmarkDetector::DetectLandmarksInVideo(captured_image, face_model, det_parameters, grayscale_image);
			
            // Gaze tracking, absolute gaze direction
			cv::Point3f rightGazeDirection(0, 0, 0); cv::Point3f leftGazeDirection(0, 0, 0); 
			cv::Vec2d rightGazeAngle(0, 0); cv::Vec2d leftGazeAngle(0, 0);
			float rightGazeDistance; float leftGazeDistance;
			cv::Point3f rightGazeCoord(0, 0, 0); cv::Point3f leftGazeCoord(0, 0, 0);
			cv::Point2f rightScreenCoord(0,0); cv::Point2f leftScreenCoord(0,0);
			cv::Point3f rightEyePoint(0, 0, 0); cv::Point3f leftEyePoint(0, 0, 0);
			cv::Point2f screen_center(0,0); cv::Point2f screen_coord(0,0);

            if (detection_success && face_model.eye_model)
			{
				GazeAnalysis::EstimateGaze(face_model, rightGazeDirection, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, true); // 오른쪽
				GazeAnalysis::EstimateGaze(face_model, leftGazeDirection, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, false);
				
				rightGazeAngle = GazeAnalysis::GetGazeAnglebyOneDir(rightGazeDirection);
				leftGazeAngle = GazeAnalysis::GetGazeAnglebyOneDir(leftGazeDirection);

				rightGazeDistance = MappingScreen::GetGazeDistance(rightGazeAngle, screen_face_distance);
				leftGazeDistance = MappingScreen::GetGazeDistance(leftGazeAngle, screen_face_distance);

				// 오른쪽 눈 (11번 랜드마크)
				rightEyePoint.x = face_model.detected_landmarks.at<float>(0, 11);
				rightEyePoint.y = face_model.detected_landmarks.at<float>(1, 11);
				rightEyePoint.z = face_model.detected_landmarks.at<float>(2, 11);

				// 왼쪽 눈 (39번 랜드마크)
				leftEyePoint.x = face_model.detected_landmarks.at<float>(0, 39);
				leftEyePoint.y = face_model.detected_landmarks.at<float>(1, 39);
				leftEyePoint.z = face_model.detected_landmarks.at<float>(2, 39);

				// 화면 교차점
				rightGazeCoord = MappingScreen::GetGazeCoord(rightEyePoint, rightGazeDirection, rightGazeDistance);
				leftGazeCoord = MappingScreen::GetGazeCoord(leftEyePoint, leftGazeDirection, leftGazeDistance);

				// 화면 중심점
				screen_center = cv::Point2f(screen_width/2, screen_height/2);  
				
				// 화면 매핑
				rightScreenCoord = MappingScreen::GetScreenCoord(rightGazeCoord, rightEyePoint, screen_center, scaling);
				leftScreenCoord = MappingScreen::GetScreenCoord(leftGazeCoord, leftEyePoint, screen_center, scaling);
				
				screen_coord = (rightScreenCoord + leftScreenCoord) / 2;
				
				// 중심과 화면 좌표 간 거리 계산
				double distance_from_center = cv::norm(screen_coord - screen_center);
				int direction = MappingScreen::calculateDirection(screen_center, screen_coord);
				
				// slopes
				rightScreenCoord = MappingScreen::GetScreenCoord(rightGazeCoord, rightEyePoint, screen_center, gazeCoord.slopes[direction]*distance_from_center + gazeCoord.intercept);
				leftScreenCoord = MappingScreen::GetScreenCoord(leftGazeCoord, leftEyePoint, screen_center, gazeCoord.slopes[direction]* distance_from_center + gazeCoord.intercept);
				
				screen_coord = (rightScreenCoord + leftScreenCoord) / 2;
				
				kf.predict();
				kf.correct(screen_coord);
				screen_coord = kf.getCorrectedPosition();
				screen_coord = clampToScreen(screen_coord);
				if (gazeCoord.getIsPredictMode()){
					// dwell time 확인
					clickManager.updateDwellTime(screen_coord, gazeCoord);
				}
				// coordinateSeqeunce에 추가
				gazeCoord.updateSequence(screen_coord);

			}

            // Keeping track of FPS
			fps_tracker.AddFrame();
			
			ui.SetImage(captured_image, screen_width, screen_height);
			ui.SetRedScreenCoord(screen_coord);
			ui.SetGrid(screen_width, screen_height, GRID_SIZE);

            // Grabbing the next frame in the sequence
			captured_image = sequence_reader.GetNextFrame();

        }

        INFO_STREAM("Closing output recorder");
		sequence_reader.Close();
		INFO_STREAM("Closed successfully");

		// Reset the models for the next video
		face_model.Reset();

    }

    return 0;
}

// 시선 추적 작업을 백그라운드에서 실행하는 함수
void StartGazeTrackingInBackground(int argc, char **argv) {
    std::thread gazeTrackingThread([=]() {
        runGazeClickerTasks(argc, argv);  // 시선 추적 처리
        std::cout << "Gaze tracking started in background thread." << std::endl;
    });
    gazeTrackingThread.detach();  // 백그라운드에서 독립적으로 실행
}

int main(int argc, char **argv){

	@autoreleasepool {
        // 시선 추적 작업을 백그라운드에서 실행
        StartGazeTrackingInBackground(argc, argv);
        
        // 상태바 애플리케이션을 메인 스레드에서 실행
        CreateStatusBarApp();  
    }
    return 0;

}