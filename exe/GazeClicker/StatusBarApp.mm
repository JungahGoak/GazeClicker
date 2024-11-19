// StatusBarApp.mm
#include <opencv2/opencv.hpp>
#import <Cocoa/Cocoa.h>
#include "UI.h"
#include "GazeCoordinate.h"
#include "GazeClickerConfig.h"

// 상태 표시줄 아이템과 카메라 창을 위한 전역 변수
NSStatusItem *statusItem;
NSWindow *cameraWindow;
NSImageView *cameraView;
NSTimer *cameraTimer;
NSWindow *cameraPreviewWindow;
NSImageView *cameraPreviewView;
NSTimer *redCircleTimer;  // 빨간 동그라미 타이머
NSWindow *overlayWindow;  // 빨간 동그라미 오버레이 창

extern GazeCoordinate::GazeCoordinate gazeCoord;

// RedCircleView 정의: 오직 빨간 동그라미만 표시
@interface RedCircleView : NSView
@property (nonatomic, assign) NSPoint circlePosition;
@end

@implementation RedCircleView
- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    CGFloat gridWidth = self.bounds.size.width / 10.0;
    CGFloat gridHeight = self.bounds.size.height / 10.0;

    // 10x10 그리드 그리기
    for (int i = 0; i <= 10; i++) {
        // 수평선 그리기
        NSBezierPath *horizontalLine = [NSBezierPath bezierPath];
        [horizontalLine moveToPoint:NSMakePoint(0, i * gridHeight)];
        [horizontalLine lineToPoint:NSMakePoint(self.bounds.size.width, i * gridHeight)];
        [[NSColor lightGrayColor] setStroke];
        [horizontalLine stroke];

        // 수직선 그리기
        NSBezierPath *verticalLine = [NSBezierPath bezierPath];
        [verticalLine moveToPoint:NSMakePoint(i * gridWidth, 0)];
        [verticalLine lineToPoint:NSMakePoint(i * gridWidth, self.bounds.size.height)];
        [[NSColor lightGrayColor] setStroke];
        [verticalLine stroke];
    }

    // 각 그리드에 번호 표시
    NSDictionary *textAttributes = @{NSFontAttributeName: [NSFont systemFontOfSize:10],
                                      NSForegroundColorAttributeName: [NSColor blackColor]};
    int number = 0;
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            CGFloat x = col * gridWidth + gridWidth / 2.0;
            CGFloat y = self.bounds.size.height - (row * gridHeight + gridHeight / 2.0);
            NSString *numberString = [NSString stringWithFormat:@"%d", number];
            CGSize textSize = [numberString sizeWithAttributes:textAttributes];
            NSPoint textPoint = NSMakePoint(x - textSize.width / 2.0, y - textSize.height / 2.0);
            [numberString drawAtPoint:textPoint withAttributes:textAttributes];
            number++;
        }
    }


    if (!gazeCoord.getIsPredictMode()) {
        // 모든 좌표를 화면에 표시
        for (cv::Point2f coord : gazeCoord.coord_sequence) {
            NSRect pointRect = NSMakeRect(coord.x - 5, screen_height - coord.y - 5, 10, 10);
            [[NSColor blueColor] setFill];
            [[NSBezierPath bezierPathWithOvalInRect:pointRect] fill];
        }
    }

    // 원의 위치와 크기 설정
    NSRect circleRect = NSMakeRect(self.circlePosition.x - 15, screen_height - self.circlePosition.y + 15, 30, 30);

    if (gazeCoord.getIsPredictMode() && gazeCoord.getIsClickTrigger()) {
        // 왼쪽 반쪽 노란색 화살표와 "Yes" 텍스트
        [[NSColor yellowColor] setFill];
        NSRect leftHalf = NSMakeRect(circleRect.origin.x, circleRect.origin.y, circleRect.size.width / 2, circleRect.size.height);
        
        // 화살표 그리기
        NSBezierPath *leftArrow = [NSBezierPath bezierPath];
        [leftArrow moveToPoint:NSMakePoint(leftHalf.origin.x + leftHalf.size.width - 3, leftHalf.origin.y)];
        [leftArrow lineToPoint:NSMakePoint(leftHalf.origin.x + leftHalf.size.width - 3, leftHalf.origin.y + leftHalf.size.height)];
        [leftArrow lineToPoint:NSMakePoint(leftHalf.origin.x + 3, leftHalf.origin.y + leftHalf.size.height / 2)];
        [leftArrow closePath];
        [leftArrow fill];
        
        // "Yes" 텍스트 추가 (화살표 위에)
        NSDictionary *yesAttributes = @{NSFontAttributeName: [NSFont systemFontOfSize:12],
                                        NSForegroundColorAttributeName: [NSColor yellowColor]};
        NSString *yesText = @"Yes";
        [yesText drawAtPoint:NSMakePoint(leftHalf.origin.x - 10, leftHalf.origin.y + leftHalf.size.height) withAttributes:yesAttributes];

        // 오른쪽 반쪽 빨간색 화살표와 "No" 텍스트
        [[NSColor redColor] setFill];
        NSRect rightHalf = NSMakeRect(circleRect.origin.x + circleRect.size.width / 2, circleRect.origin.y, circleRect.size.width / 2, circleRect.size.height);
        
        // 화살표 그리기
        NSBezierPath *rightArrow = [NSBezierPath bezierPath];
        [rightArrow moveToPoint:NSMakePoint(rightHalf.origin.x + 3, rightHalf.origin.y)];
        [rightArrow lineToPoint:NSMakePoint(rightHalf.origin.x + 3, rightHalf.origin.y + rightHalf.size.height)];
        [rightArrow lineToPoint:NSMakePoint(rightHalf.origin.x + rightHalf.size.width - 3, rightHalf.origin.y + rightHalf.size.height / 2)];
        [rightArrow closePath];
        [rightArrow fill];
        
        // "No" 텍스트 추가 (화살표 위에)
        NSDictionary *noAttributes = @{NSFontAttributeName: [NSFont systemFontOfSize:12],
                                       NSForegroundColorAttributeName: [NSColor redColor]};
        NSString *noText = @"No";
        [noText drawAtPoint:NSMakePoint(rightHalf.origin.x + 10, rightHalf.origin.y + rightHalf.size.height) withAttributes:noAttributes];
    } else {
        // 기본 빨간색 원
        [[NSColor redColor] setFill];
        [[NSBezierPath bezierPathWithOvalInRect:circleRect] fill];
    }
}
@end

RedCircleView *redCircleView;  // RedCircleView 인스턴스

void ToggleMode(NSMenuItem *sender) {
    // 모드를 토글
    gazeCoord.setIsPredictMode(!gazeCoord.getIsPredictMode());
    sender.title = gazeCoord.getIsPredictMode() ? @"learn mode" : @"predict mode";
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

// 투명한 오버레이 윈도우 생성 함수
void CreateOverlayWindow() {
    NSScreen *mainScreen = [NSScreen mainScreen];
    NSRect screenFrame = [mainScreen frame];

    overlayWindow = [[NSWindow alloc] initWithContentRect:screenFrame
                                                styleMask:NSWindowStyleMaskBorderless
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    [overlayWindow setBackgroundColor:[NSColor clearColor]];
    [overlayWindow setOpaque:NO];  // 창을 투명하게
    [overlayWindow setIgnoresMouseEvents:YES];  // 마우스 이벤트 무시
    [overlayWindow setLevel:NSFloatingWindowLevel]; // 모든 창 위에 표시
    
    // RedCircleView를 overlayWindow에 추가
    redCircleView = [[RedCircleView alloc] initWithFrame:screenFrame];
    redCircleView.wantsLayer = YES;
    [[overlayWindow contentView] addSubview:redCircleView];

    [overlayWindow makeKeyAndOrderFront:nil];
}

// 빨간 동그라미 위치를 업데이트하는 함수
void UpdateRedCirclePosition() {
    if (gazeCoord.coord_sequence.size() > 0) {
        cv::Point2f screenCoord = gazeCoord.coord_sequence.back();
        redCircleView.circlePosition = NSMakePoint(screenCoord.x, screenCoord.y);
        [redCircleView setNeedsDisplay:YES];
    }
}

// 카메라 창에 주기적으로 프레임을 업데이트하는 함수
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

    NSInteger width = rgbaMat.cols;
    NSInteger height = rgbaMat.rows;

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

    memcpy([imgRep bitmapData], rgbaMat.data, rgbaMat.total() * rgbaMat.elemSize());

    NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
    [image addRepresentation:imgRep];

    return image;
}

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
    if (gazeCoord.coord_sequence.size() > 0) {
        cv::Point2f screen_coord = gazeCoord.coord_sequence.back();

        // RedCircleView에 새 위치 업데이트 및 표시
        redCircleView.circlePosition = NSMakePoint(screen_coord.x, screen_coord.y);
        [redCircleView setNeedsDisplay:YES];
    }
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate
- (void)toggleMode:(id)sender {
    ToggleMode((NSMenuItem *)sender);
}
- (void)quitApp:(id)sender {
    if (cameraTimer) {
        [cameraTimer invalidate];
        cameraTimer = nil;
    }
    if (cameraWindow) {
        [cameraWindow close];
        cameraWindow = nil;
    }
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


void CreateStatusBarApp() {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *appDelegate = [[AppDelegate alloc] init];
        [app setDelegate:appDelegate];
        
        statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        statusItem.button.title = @"GazeClicker";
        
        NSMenu *menu = [[NSMenu alloc] init];

        NSMenuItem *toggleModeItem = [[NSMenuItem alloc] initWithTitle:@"predict mode"
                                                                action:@selector(toggleMode:)
                                                         keyEquivalent:@"m"];
        [toggleModeItem setTarget:appDelegate];

        NSMenuItem *openCameraItem = [[NSMenuItem alloc] initWithTitle:@"Open Camera"
                                                                action:@selector(showCameraWindow:)
                                                         keyEquivalent:@"c"];
        [openCameraItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
        [openCameraItem setTarget:appDelegate];

        NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                          action:@selector(quitApp:)
                                                   keyEquivalent:@"q"];
        [quitItem setTarget:appDelegate];
        
        [menu addItem:toggleModeItem];
        [menu addItem:openCameraItem];
        [menu addItem:quitItem];
        statusItem.menu = menu;
        
        // 타이머 설정으로 주기적으로 호출
        cameraTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/40.0
                                                        target:[NSBlockOperation blockOperationWithBlock:^{
                                                            UpdateCameraFrame();
                                                            UpdateStatusBarWithCoord();
                                                        }]
                                                    selector:@selector(main)
                                                    userInfo:nil
                                                    repeats:YES];

        // 투명한 오버레이 윈도우 생성 및 빨간 동그라미 위치 업데이트
        CreateOverlayWindow();
        redCircleTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 40.0
                                                          target:[NSBlockOperation blockOperationWithBlock:^{
                                                              UpdateRedCirclePosition();
                                                          }]
                                                        selector:@selector(main)
                                                        userInfo:nil
                                                         repeats:YES];

        [app setActivationPolicy:NSApplicationActivationPolicyAccessory];
        [app run];
    }
}
