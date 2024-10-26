# 필요한 라이브러리 경로 포함
include_directories(../../local/LandmarkDetector/include)
include_directories(../../local/Utilities/include)
include_directories(../../local/MappingScreen/include)  # MappingScreen 경로 포함

# 소스 파일 설정
SET(SOURCE
    src/HMM.cpp
    src/GazePattern.cpp
    ../../local/MappingScreen/src/MappingScreen.cpp  # MappingScreen 소스 파일 포함
)

# 헤더 파일 설정
SET(HEADERS
    include/HMM.h
    include/GazePattern.h
    include/GazeClickerConfig.h
    ../../local/MappingScreen/include/MappingScreen.h  # MappingScreen 헤더 포함
)

# GazePattern 라이브러리 생성
add_library(GazePattern ${SOURCE} ${HEADERS})
add_library(OpenFace::GazePattern ALIAS GazePattern)

# 타겟에 경로 포함
target_include_directories(GazePattern PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>
)

# 추가 경로와 라이브러리 링크
target_include_directories(GazePattern PRIVATE ${MAPPINGSCREEN_SOURCE_DIR}/include)
target_include_directories(GazePattern PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(GazePattern PUBLIC ${OpenCV_LIBS} ${OpenBLAS_LIB} dlib::dlib)
target_include_directories(GazePattern PRIVATE ${OpenBLAS_INCLUDE_DIR})

# 설치 설정
install(TARGETS GazePattern EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/OpenFace)
