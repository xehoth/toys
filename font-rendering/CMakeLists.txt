cmake_minimum_required(VERSION 3.10)
project(font-rendering LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME freetype
  GIT_REPOSITORY https://github.com/aseprite/freetype2.git
  GIT_TAG VER-2-10-0
  VERSION 2.10.0
)

if (freetype_ADDED)
  add_library(Freetype::Freetype ALIAS freetype)
endif()

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

include_directories(include)
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_executable(main ${SRC_FILES})
target_link_libraries(main PRIVATE Freetype::Freetype glfw glad::glad)
