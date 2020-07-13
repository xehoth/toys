cmake_minimum_required(VERSION 3.18)
project(PixelGui)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

file(GLOB SRC_FILES *.cpp)
add_executable(main ${SRC_FILES})
target_link_libraries(main PRIVATE glad::glad glfw OpenMP::OpenMP_CXX)