cmake_minimum_required(VERSION 3.2)
project(Fitbit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_subdirectory(lib/cpr)

include_directories (include ${LIBUSB_1_INCLUDE_DIRS} ${CPR_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(Fitbit ${SOURCE_FILES})
target_link_libraries(Fitbit usb-1.0 ${Boost_Libraries} ${CPR_LIBRARIES})