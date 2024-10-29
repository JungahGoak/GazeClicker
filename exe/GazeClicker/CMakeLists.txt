# CMakeLists.txt (exe/GazeClicker)

cmake_minimum_required(VERSION 3.8)
project(GazeClicker)

set(CMAKE_CXX_STANDARD 17)

SET(SOURCE
    GazeClicker.cpp
    ../../lib/local/GazePattern/src/GazeClickerConfig.cpp # GazeClickerConfig.cpp 추가
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

# macOS 필수 라이브러리 링크
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(COREGRAPHICS_LIBRARY CoreGraphics)

target_link_libraries(GazeClicker PUBLIC
    ${COREFOUNDATION_LIBRARY}
    ${COREGRAPHICS_LIBRARY}
)

# 실행 파일 설치 경로
install(TARGETS GazeClicker DESTINATION bin)
