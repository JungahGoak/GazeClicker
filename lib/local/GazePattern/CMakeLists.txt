#LandmarkDetector library
include_directories(../../local/LandmarkDetector/include)

#Utilities library
include_directories(../../local/Utilities/include)

SET(SOURCE
    src/HMMGazePattern.cpp
)

SET(HEADERS
    include/GazePattern.h
)

add_library( GazePattern ${SOURCE} ${HEADERS})
add_library( OpenFace::GazePattern ALIAS GazePattern)

target_include_directories(GazePattern PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>)

target_include_directories(GazePattern PRIVATE ${MAPPINGSCREEN_SOURCE_DIR}/include)
target_include_directories(GazePattern PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(GazePattern PUBLIC ${OpenCV_LIBS} ${OpenBLAS_LIB})
target_link_libraries(GazePattern PUBLIC dlib::dlib)

target_include_directories(GazePattern PRIVATE ${OpenBLAS_INCLUDE_DIR})

install (TARGETS GazePattern EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include/OpenFace)