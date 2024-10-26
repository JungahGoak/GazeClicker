#LandmarkDetector library
include_directories(../../local/LandmarkDetector/include)

#Utilities library
include_directories(../../local/Utilities/include)

#GazePattern library
include_directories(../../local/GazePattern/include)

SET(SOURCE
    src/MappingScreen.cpp
)

SET(HEADERS
    include/MappingScreen.h
)

add_library( MappingScreen ${SOURCE} ${HEADERS})
add_library( OpenFace::MappingScreen ALIAS MappingScreen)

target_include_directories(MappingScreen PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>)

target_include_directories(MappingScreen PRIVATE ${MAPPINGSCREEN_SOURCE_DIR}/include)
target_include_directories(MappingScreen PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(MappingScreen PUBLIC ${OpenCV_LIBS} ${OpenBLAS_LIB})
target_link_libraries(MappingScreen PUBLIC dlib::dlib)

target_include_directories(MappingScreen PRIVATE ${OpenBLAS_INCLUDE_DIR})

install (TARGETS MappingScreen EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include/OpenFace)