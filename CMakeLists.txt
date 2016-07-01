cmake_minimum_required(VERSION 3.2)
project(Fitbit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_library(libusb-1.0 REQUIRED)
include_directories (${PROJECT_SOURCE_DIR}/../include   ${LIBUSB_1_INCLUDE_DIRS})

FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

set(SOURCE_FILES Main.cpp)
add_executable(Fitbit ${SOURCE_FILES})
target_link_libraries(Fitbit usb-1.0 ${Boost_Libraries})