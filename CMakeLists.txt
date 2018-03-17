cmake_minimum_required(VERSION 3.9)
project(DuckHunt)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(SOURCE_FILES main.cpp)
add_executable(DuckHunt ${SOURCE_FILES})

target_link_libraries(DuckHunt SDL2main SDL2_image SDL2)

set(directory textures)
file(MAKE_DIRECTORY ${directory})