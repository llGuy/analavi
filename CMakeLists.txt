cmake_minimum_required(VERSION "3.2")

project(analavi)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCES "imgui/lib/*.cpp" "imgui/include/*.h", "source/*.cpp" "source/*.hpp")

include_directories("${CMAKE_SOURCE_DIR}/imgui/include")
include_directories("${OpenCV_INCLUDE_DIRS}")
include_directories("${SDL2_INCLUDE_DIRS}")

link_libraries("SDL2" "GLEW" "GL" "stdc++fs")
link_libraries("${OpenCV_LIBS}")
link_libraries("${SDL2_LIBS}")

# add_compile_options("-std=c++17" "-lstdc++fs")

add_executable(analavi "${SOURCES}")
