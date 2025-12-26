cmake_minimum_required(VERSION 3.10)
project(C++MotionSensor)

#Resolving editor errors
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Finding OpenCV Packages needed
find_package(OpenCV REQUIRED)

#Include Directories from OpenCV, needed for C/C++
include_directories(${OpenCV_INCLUDE_DIRS})

#creating executable
add_executable(C++MotionSensor main.cpp)

#Linking OpenCV Libraries
target_link_libraries(C++MotionSensor ${OpenCV_LIBS})


