cmake_minimum_required(VERSION 3.18)
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.18)
endif()
project(normal_mapping VERSION 0.1 DESCRIPTION "normal mapping test" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

aux_source_directory(. SRC_FILES)

add_executable(main ${SRC_FILES})
find_path(STB_INCLUDE_DIRS "stb.h")
target_include_directories(main PRIVATE ${STB_INCLUDE_DIRS})

target_link_libraries(main PRIVATE glad::glad glfw glm)
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shader $<TARGET_FILE_DIR:main>)
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/texture $<TARGET_FILE_DIR:main>)