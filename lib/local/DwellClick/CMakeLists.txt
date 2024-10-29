# 필요한 라이브러리 경로 포함
include_directories(../../local/LandmarkDetector/include)
include_directories(../../local/Utilities/include)
include_directories(../../local/MappingScreen/include)
include_directories(../../local/GazePattern/include)
include_directories(../../local/GazeCoordinate/include)

# 소스 파일 설정
SET(SOURCE
    src/Click.cpp
)

# 헤더 파일 설정
SET(HEADERS
    include/Click.h
)

# DwellClick 라이브러리 생성
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
target_link_libraries(DwellClick PUBLIC ${OpenCV_LIBS} ${OpenBLAS_LIB} dlib::dlib)
target_include_directories(DwellClick PRIVATE ${OpenBLAS_INCLUDE_DIR})

# 설치 설정
install(TARGETS DwellClick EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/OpenFace)
