# CMakeLists.txt (exe/GazeClicker)

cmake_minimum_required(VERSION 3.8)
project(GazeClicker)

set(CMAKE_CXX_STANDARD 17)

SET(SOURCE
    GazeClicker.mm
    StatusBarApp.mm
    ../../lib/local/GazePattern/src/GazeClickerConfig.cpp
)

# 실행 파일 생성
add_executable(GazeClicker ${SOURCE})

# include 디렉터리 설정
target_include_directories(GazeClicker PUBLIC
    ${CMAKE_SOURCE_DIR}/../../lib/local/GazePattern/include
    ${CMAKE_SOURCE_DIR}/../../lib/local/GazeCoordiante/include
    ${CMAKE_SOURCE_DIR}/../../lib/local/DwellClick/include
    ${CMAKE_SOURCE_DIR}/../../lib/local/MappingScreen/include
    ${CMAKE_SOURCE_DIR}/../../lib/local/LandmarkDetector/include
    ${CMAKE_SOURCE_DIR}/../../lib/local/Utilities/include
)

# 라이브러리 링크 설정
target_link_libraries(GazeClicker PUBLIC
    LandmarkDetector
    FaceAnalyser
    GazeAnalyser
    Utilities
    MappingScreen
    GazePattern
    GazeCoordinate
    DwellClick
)

link_directories(/usr/local/Cellar/jpeg-xl/0.11.0_1/lib)

# macOS 필수 라이브러리 링크
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(COREGRAPHICS_LIBRARY CoreGraphics)
find_library(COCOA_LIBRARY Cocoa)
find_library(AVFOUNDATION_LIBRARY AVFoundation)  # AVFoundation 추가

target_link_libraries(GazeClicker PUBLIC
    ${COREFOUNDATION_LIBRARY}
    ${COREGRAPHICS_LIBRARY}
    ${COCOA_LIBRARY}
    ${AVFOUNDATION_LIBRARY}
)