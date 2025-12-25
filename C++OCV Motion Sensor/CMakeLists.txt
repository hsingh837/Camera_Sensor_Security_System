cmake_minimum_required(VERSION 3.10)
project(C++OCV Motion Sensor)

#Find OpenCV packages necessary
find_package(OpenCV REQUIRED)


#Include necessary directories
include_directories(${OpenCV_INCLUDE_DIRS})


#Create an Executable
add_executable(C++OCV Motion Sensor main.cpp)


#Linking OpenCV Libraries
target_link_libraries(C++OCV Motion Sensor ${OPENCV_LIBS})