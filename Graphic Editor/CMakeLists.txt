cmake_minimum_required(VERSION 3.10)
project(GraphicEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")

add_executable(GraphicEditor ${SOURCES})

target_link_libraries(GraphicEditor ${SDL2_LIBRARIES})
