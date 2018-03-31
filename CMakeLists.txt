cmake_minimum_required(VERSION 3.10)
project(course_work)

set(CMAKE_CXX_STANDARD 11)

# includes cmake/FindSDL2.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(course_work main.cpp)
target_link_libraries(course_work ${SDL2_LIBRARY})
