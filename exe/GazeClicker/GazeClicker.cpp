// GazeClicker.cpp : 

// Local includes
#include "LandmarkCoreIncludes.h"

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

#include <ApplicationServices/ApplicationServices.h> // Quartz Event Services를 위한 헤더

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

	// First argument is reserved for the name of the executable
	for (int i = 0; i < argc; ++i)
	{
		arguments.push_back(std::string(argv[i]));
	}
	return arguments;
}

// HMM GazePattern
int screen_width = 2560;
int screen_height = 1664;
GazePattern::HMMGazePattern gazePattern(screen_width, screen_height);

std::deque<cv::Point2f> coordSequence;
const int coordinate_sequence_size = 10; //sequence 길이

void updateSequence(cv::Point2f newCoord) {
    if (coordSequence.size() >= coordinate_sequence_size) {
        coordSequence.pop_front();  // 가장 오래된 좌표를 제거
    }
    coordSequence.push_back(newCoord);  // 새로운 좌표 추가
}

// 마우스 이벤트 콜백 함수
CGEventRef mouseCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
    if (type == kCGEventLeftMouseDown) {

		// 클릭 좌표
        CGPoint mouseLocation = CGEventGetLocation(event);
		
		// HMM parameter update (click event)
		/*
        if (coordSequence.size() == coordinate_sequence_size) {
            gazePattern.updateHMMParameters(coordSequence, coordinate_sequence_size, cv::Point2f(mouseLocation.x, mouseLocation.y));
        } else {
            std::cerr << "Sequence size mismatch. Current sequence size: " << coordSequence.size() << ", Expected: " << coordinate_sequence_size << std::endl;
        }
		*/
		
		std::cout << "Mouse clicked at (" << mouseLocation.x << ", " << mouseLocation.y << ")" << std::endl;
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
		nullptr);

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

int main(int argc, char **argv){

	std::vector<std::string> arguments = get_arguments(argc, argv);

	// UI 생성
	Utilities::UI ui(arguments);
	ui.CreateTrackbars();
	ui.ShowUI();

	// 트랙바의 "확인" 버튼이 눌릴 때까지 대기
    while (!ui.IsConfirmed()) {
        cv::waitKey(100);  // 100ms 간격으로 확인 여부 체크
    }

	// User 입력 변수
	float scaling = ui.scaling;
	double screen_face_distance = ui.screen_face_distance;

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
	//Utilities::Visualizer visualizer(arguments);

	// Tracking FPS for visualization
	Utilities::FpsTracker fps_tracker;
	fps_tracker.AddFrame();

	// kalman filter
	Utilities::KalmanFilter kf;

	// 마우스 이벤트 루프를 별도의 스레드에서 실행
    std::thread mouseEventThread(startMouseEventLoop);
	// 스레드를 detach하여 메인 스레드가 마우스 이벤트를 기다리지 않게 함
    mouseEventThread.detach();
	

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
			//visualizer.vis_track = true;
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
				
				rightScreenCoord = MappingScreen::GetScreenCoord(rightGazeCoord, rightEyePoint, screen_center, scaling);
				leftScreenCoord = MappingScreen::GetScreenCoord(leftGazeCoord, leftEyePoint, screen_center, scaling);
				
				screen_coord = (rightScreenCoord + leftScreenCoord) / 2;
				// screen_coord 출력
    			//std::cout << "Screen Coord: (" << screen_coord.x << ", " << screen_coord.y << ")" << std::endl;

				// [kalman filter] 예측 단계
				cv::Point2f prediction = kf.predict();
				
				// [kalman filter] 갱신 단계
				kf.correct(screen_coord);
				screen_coord = kf.getCorrectedPosition();
				
				// coordinateSeqeunce에 추가
				updateSequence(screen_coord);
			}

            // Keeping track of FPS
			fps_tracker.AddFrame();

            //visualizer.SetImage(captured_image, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, screen_width, screen_height);
			ui.SetImage(captured_image, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, screen_width, screen_height);
			ui.SetScreenCoord(rightScreenCoord, leftScreenCoord, screen_coord);
			//visualizer.SetFps(fps_tracker.GetFPS());
			//visualizer.SetObservationGaze(gazeDirection0, gazeDirection1, LandmarkDetector::CalculateAllEyeLandmarks(face_model), LandmarkDetector::Calculate3DEyeLandmarks(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy), face_model.detection_certainty);
			//visualizer.SetScreenCoord(rightScreenCoord, leftScreenCoord, screen_coord);

			char ui_press = ui.ShowTrack();

            // detect key presses
			//char character_press = visualizer.ShowObservation();
			
			// quit processing the current sequence (useful when in Webcam mode)
			if (ui_press == 'q')
			{
				break;
			}

			// HMM predict
			if (ui_press == 'p')
			{
				if (coordSequence.size() == coordinate_sequence_size)
				{
					int predictedRegion = gazePattern.predictHMM(coordSequence, coordinate_sequence_size);
					std::cout << "Predicted region using HMM: " << predictedRegion << std::endl;
					ui.ShowCoord(cv::Point2f(predictedRegion/10, predictedRegion%10));
				}
			}

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