
cmake_minimum_required(VERSION 3.16)
project(camera_grab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Locate OpenCV installation
find_package(OpenCV REQUIRED)
# Build main.cpp as an executable
add_executable(camera_grab main.cpp
        CameraInput.cpp
        CameraInput.h
        RegionAnalyzer.cpp
        ContourDetection.cpp
        ContourDetection.h
        LineEstimation.cpp
        LineEstimation.h
        IsValidQuad.cpp
        IsValidQuad.h)
# Give the program access to OpenCV headers and libraries.
target_include_directories(camera_grab PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(camera_grab PRIVATE ${OpenCV_LIBS})

# Unit tests for RegionAnalyzer. ctest || ./test_region_analyzer
enable_testing()
add_executable(test_region_analyzer tests/test_region_analyzer.cpp RegionAnalyzer.cpp)
target_include_directories(test_region_analyzer PRIVATE ${CMAKE_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_region_analyzer PRIVATE ${OpenCV_LIBS})
add_test(NAME region_analyzer COMMAND test_region_analyzer)
