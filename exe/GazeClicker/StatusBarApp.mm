// StatusBarApp.mm
#include <opencv2/opencv.hpp>
#import <Cocoa/Cocoa.h>
#include "UI.h"
#include "GazeCoordinate.h"

// 상태 표시줄 아이템과 카메라 창을 위한 전역 변수
NSStatusItem *statusItem;
NSWindow *cameraWindow;
NSImageView *cameraView;
NSTimer *cameraTimer;
NSWindow *cameraPreviewWindow;
NSImageView *cameraPreviewView;
NSWindow *popupWindow;
NSTextField *popupMessage;
NSButton *yesButton;
NSButton *noButton;

extern GazeCoordinate::GazeCoordinate gazeCoord;

// yes,no Popup 창 생성 및 설정
void CreatePopupWindow() {
    if (gazeCoord.coord_sequence.size() > 0) {
        cv::Point2f screen_coord = gazeCoord.coord_sequence.back();
        NSPoint popupPosition = NSMakePoint(screen_coord.x, screen_coord.y);

        popupWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(popupPosition.x, popupPosition.y, 200, 100)
                                                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                                    backing:NSBackingStoreBuffered
                                                    defer:NO];
        [popupWindow setTitle:@"Confirm Action"];

        popupMessage = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 50, 160, 20)];
        [popupMessage setStringValue:@"Trigger detected! Confirm action?"];
        [popupMessage setEditable:NO];
        [popupMessage setBezeled:NO];
        [popupMessage setDrawsBackground:NO];
        [[popupWindow contentView] addSubview:popupMessage];

        yesButton = [[NSButton alloc] initWithFrame:NSMakeRect(20, 10, 80, 30)];
        [yesButton setTitle:@"Yes"];
        [yesButton setTarget:NSApp];
        //[yesButton setAction:@selector(handleYes)];
        [[popupWindow contentView] addSubview:yesButton];

        noButton = [[NSButton alloc] initWithFrame:NSMakeRect(100, 10, 80, 30)];
        [noButton setTitle:@"No"];
        [noButton setTarget:NSApp];
        //[noButton setAction:@selector(handleNo)];
        [[popupWindow contentView] addSubview:noButton];
    }
}

// 카메라 창 생성 및 설정
void CreateCameraWindow() {
    NSScreen *screen = [NSScreen mainScreen];
    NSRect screenRect = [screen frame]; // 화면 전체 크기 가져오기

    cameraWindow = [[NSWindow alloc] initWithContentRect:screenRect
                                               styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];
    [cameraWindow setTitle:@"Camera View"];
    [cameraWindow setLevel:NSStatusWindowLevel]; // 창을 다른 창 위로 표시

    cameraView = [[NSImageView alloc] initWithFrame:[[cameraWindow contentView] bounds]];
    [cameraView setImageScaling:NSImageScaleAxesIndependently];
    [cameraWindow setContentView:cameraView];

    // 투명도 설정
    [cameraWindow setOpaque:NO];           // 창의 불투명도 설정 해제
    [cameraWindow setAlphaValue:0.2];      // 투명도 설정 (50%)
    
    [cameraWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [cameraWindow toggleFullScreen:nil]; // 전체 화면 모드로 전환
    [cameraWindow makeKeyAndOrderFront:nil];

    // 창이 닫힐 때 이벤트 핸들러 추가
    [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillCloseNotification
                                                      object:cameraWindow
                                                       queue:nil
                                                  usingBlock:^(NSNotification *note) {
                                                      cameraWindow = nil; // 창이 닫혔을 때 포인터 해제
                                                  }];
}

NSImage* ConvertMatToNSImage(const cv::Mat& mat) {
    // OpenCV의 BGR 이미지를 RGBA로 변환
    cv::Mat rgbaMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbaMat, cv::COLOR_BGR2RGBA);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, rgbaMat, cv::COLOR_GRAY2RGBA);
    } else {
        rgbaMat = mat;
    }

    // 이미지 크기 정보 설정
    NSInteger width = rgbaMat.cols;
    NSInteger height = rgbaMat.rows;

    // RGBA 데이터를 기반으로 NSBitmapImageRep 생성
    NSBitmapImageRep* imgRep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:NULL
                      pixelsWide:width
                      pixelsHigh:height
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSDeviceRGBColorSpace
                    bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                     bytesPerRow:width * 4
                    bitsPerPixel:32];

    // OpenCV 데이터를 NSBitmapImageRep로 복사
    memcpy([imgRep bitmapData], rgbaMat.data, rgbaMat.total() * rgbaMat.elemSize());

    // NSImage 생성 및 이미지 설정
    NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
    [image addRepresentation:imgRep];

    return image;
}

// 카메라 창에 주기적으로 프레임을 업데이트하는 함수
void UpdateCameraFrame() {
    if (!gazeCoord.ui.captured_image.empty()) {
        NSImage *image = ConvertMatToNSImage(gazeCoord.ui.captured_image);
        [cameraView setImage:image];
        [cameraPreviewView setImage:image];
    } else {
        std::cerr << "Camera frame is empty!" << std::endl;
    }
}

void UpdateStatusBarWithCoord() {
    if (gazeCoord.getIsClickTrigger() == true) {
        if (!popupWindow) {
            CreatePopupWindow();
        }
        [popupWindow makeKeyAndOrderFront:nil];  // 팝업 창 표시
    }

    // gazeCoord가 초기화되었고, coord_sequence가 비어 있지 않은지 확인
    if (gazeCoord.coord_sequence.size() > 0) {
        cv::Point2f screen_coord = gazeCoord.coord_sequence.back();
        NSString *coordText = [NSString stringWithFormat:@"X: %.2f, Y: %.2f", screen_coord.x, screen_coord.y];
        statusItem.button.title = coordText; // StatusBar에 표시
    } else {
        statusItem.button.title = @"No Coord Data"; // 데이터가 없는 경우 표시
    }
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate
- (void)quitApp:(id)sender {
    // Stop the timer
    if (cameraTimer) {
        [cameraTimer invalidate];
        cameraTimer = nil;
    }
    // Close any open windows
    if (popupWindow) {
        [popupWindow close];
        popupWindow = nil;
    }
    if (cameraWindow) {
        [cameraWindow close];
        cameraWindow = nil;
    }
    // Terminate the application
    [[NSApplication sharedApplication] terminate:nil];
}

- (void)showCameraWindow:(id)sender {
    if (!cameraWindow) {
        CreateCameraWindow();
    } else {
        [cameraWindow makeKeyAndOrderFront:nil];
    }
}
@end

// 상태 표시줄 애플리케이션 생성
void CreateStatusBarApp() {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *appDelegate = [[AppDelegate alloc] init];
        [app setDelegate:appDelegate];
        
        // 상태 표시줄에 아이콘 추가
        statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        statusItem.button.title = @"GazeClicker";
        
        // 상태바 메뉴 설정
        NSMenu *menu = [[NSMenu alloc] init];
        NSMenuItem *openCameraItem = [[NSMenuItem alloc] initWithTitle:@"Open Camera"
                                                                action:@selector(showCameraWindow:)
                                                         keyEquivalent:@""];
        [openCameraItem setTarget:appDelegate];

        NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                          action:@selector(quitApp:)
                                                   keyEquivalent:@"q"];
        [quitItem setTarget:appDelegate];

        [menu addItem:openCameraItem];
        [menu addItem:quitItem];
        statusItem.menu = menu;
        
        // 타이머 설정으로 주기적으로 호출
        cameraTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                                        target:[NSBlockOperation blockOperationWithBlock:^{
                                                            UpdateCameraFrame();
                                                            UpdateStatusBarWithCoord();
                                                        }]
                                                    selector:@selector(main)
                                                    userInfo:nil
                                                    repeats:YES];

        // 메인 윈도우 생성 및 앱 실행
        [app setActivationPolicy:NSApplicationActivationPolicyAccessory];
        [app run];
    }
}