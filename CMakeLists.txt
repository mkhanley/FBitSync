cmake_minimum_required(VERSION 3.2)
project(FBSync)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_subdirectory(lib/cpr)
set(${USE_SYSTEM_CURL} ON)

include_directories (include ${LIBUSB_1_INCLUDE_DIRS} ${CPR_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(FBSync ${SOURCE_FILES})
target_link_libraries(FBSync usb-1.0 ${Boost_Libraries} ${CPR_LIBRARIES})