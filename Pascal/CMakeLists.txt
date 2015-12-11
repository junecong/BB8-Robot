cmake_minimum_required(VERSION 2.8)
project(run)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(FSM Vision/FSM.cpp)
add_library(TRACK Vision/motionTrack.cpp)
add_executable(run Communication/send.cpp Vision/FSM.cpp Vision/motionTrack.cpp)

target_link_libraries(TRACK ${OpenCV_LIBS} FSM)
target_link_libraries(run TRACK)