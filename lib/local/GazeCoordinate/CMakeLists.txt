#LandmarkDetector library
include_directories(../../local/LandmarkDetector/include)

#Utilities library
include_directories(../../local/Utilities/include)

#GazePattern library
include_directories(../../local/GazePattern/include)

# MappingScreen library
include_directories(../../local/MappingScreen/include)

# DwellClick library
include_directories(../../local/DwellClick/include) 

SET(SOURCE
    src/GazeCoordinate.cpp
)

SET(HEADERS
    include/GazeCoordinate.h
)

add_library( GazeCoordinate ${SOURCE} ${HEADERS})
add_library( OpenFace::GazeCoordinate ALIAS GazeCoordinate)

target_include_directories(GazeCoordinate PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/OpenFace>)

target_include_directories(GazeCoordinate PRIVATE ${MAPPINGSCREEN_SOURCE_DIR}/include)
target_include_directories(GazeCoordinate PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(GazeCoordinate PUBLIC ${OpenCV_LIBS} ${OpenBLAS_LIB})
target_link_libraries(GazeCoordinate PUBLIC dlib::dlib)

target_include_directories(GazeCoordinate PRIVATE ${OpenBLAS_INCLUDE_DIR})

install (TARGETS GazeCoordinate EXPORT OpenFaceTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include/OpenFace)