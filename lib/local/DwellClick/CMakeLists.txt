# 최소 CMake 버전
cmake_minimum_required(VERSION 3.10)
project(DwellClick)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV 및 OpenBLAS 설정 (필요 시 수정)
find_package(OpenCV REQUIRED)
find_library(OpenBLAS_LIB openblas REQUIRED)

# 추가 헤더 경로 포함
include_directories(../../local/LandmarkDetector/include)
include_directories(../../local/Utilities/include)
include_directories(../../local/MappingScreen/include)
include_directories(../../local/GazePattern/include)
include_directories(../../local/GazeCoordinate/include)

# 소스 파일 설정
set(SOURCE
    src/Click.cpp
)

# 헤더 파일 설정
set(HEADERS
    include/Click.h
)

# 타겟 라이브러리 생성
add_library(DwellClick ${SOURCE} ${HEADERS})
add_library(OpenFace::DwellClick ALIAS DwellClick)

# 타겟에 경로 포함
target_include_directories(DwellClick PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>
)

# 추가 경로와 라이브러리 링크
target_include_directories(DwellClick PRIVATE ${MAPPINGSCREEN_SOURCE_DIR}/include)
target_include_directories(DwellClick PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(DwellClick PUBLIC
    ${OpenCV_LIBS}       # OpenCV 라이브러리
    ${OpenBLAS_LIB}      # OpenBLAS 라이브러리
    dlib::dlib           # Dlib 라이브러리
)

# 설치 설정
install(TARGETS DwellClick EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/OpenFace)
