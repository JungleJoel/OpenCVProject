cmake_minimum_required(VERSION 3.16)
project(camera_grab LANGUAGES CXX)

# Locate the OpenCV installation.
find_package(OpenCV REQUIRED)

# Build main.cpp as an executable named camera_grab.
add_executable(main main.cpp)

# Give the program access to OpenCV headers and libraries.
target_link_libraries(main PRIVATE ${OpenCV_LIBS})
