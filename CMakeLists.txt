cmake_minimum_required(VERSION 3.5)
project(Fitbit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_library(libusb-1.0 REQUIRED)
include_directories (${PROJECT_SOURCE_DIR}/../include   ${LIBUSB_1_INCLUDE_DIRS})

set(SOURCE_FILES Main.cpp Dongle.cpp)
add_executable(Fitbit ${SOURCE_FILES})
target_link_libraries(Fitbit usb-1.0)