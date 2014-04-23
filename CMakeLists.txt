cmake_minimum_required(VERSION 2.8)

project(BITK)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE debug)

find_package(OpenCV REQUIRED)

include_directories(OpenCV_INCLUDE_DIRS)

add_executable(BITK main.cpp)

target_link_libraries(BITK ${OpenCV_LIBRARIES})

FILE(GLOB VIDEOS videos/*.avi)
file(COPY ${VIDEOS} DESTINATION ./videos/)
