# Local libraries
include_directories(${LandmarkDetector_SOURCE_DIR}/include)
	
add_executable(GazeClicker GazeClicker.cpp)
target_link_libraries(GazeClicker LandmarkDetector)
target_link_libraries(GazeClicker FaceAnalyser)
target_link_libraries(GazeClicker GazeAnalyser)
target_link_libraries(GazeClicker Utilities)
target_link_libraries(GazeClicker MappingScreen)

install (TARGETS GazeClicker DESTINATION bin)