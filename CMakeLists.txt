cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/rct)
add_executable(diff main.cpp rct/rct/Path.cpp rct/rct/Log.cpp)
