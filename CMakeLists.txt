cmake_minimum_required(VERSION "3.2")

project(analavi)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCES "imgui/lib/*.cpp" "imgui/include/*.h", "source/*.cpp" "source/*.hpp" "ImGuiColorTextEdit/*.cpp" "ImGuiColorTextEdit/*.h")

include_directories("${CMAKE_SOURCE_DIR}/imgui/include")
include_directories("ImGuiColorTextEdit/")
include_directories("${OpenCV_INCLUDE_DIRS}")
include_directories("${SDL2_INCLUDE_DIRS}")
include_directories("lua/")

link_libraries("SDL2" "GLEW" "GL" "stdc++fs")
link_libraries("${OpenCV_LIBS}")
link_libraries("${SDL2_LIBS}")

# add_compile_options("-std=c++17" "-lstdc++fs")

add_library(lua "lua/lapi.c"
                "lua/lauxlib.c"
                "lua/lbaselib.c"
                "lua/lcode.c"
                "lua/ldblib.c"
                "lua/ldebug.c"
                "lua/ldo.c"
                "lua/ldump.c"
                "lua/lfunc.c"
                "lua/lgc.c"
                "lua/linit.c"
                "lua/liolib.c"
                "lua/llex.c"
                "lua/lmathlib.c"
                "lua/lmem.c"
                "lua/loadlib.c"
                "lua/lobject.c"
                "lua/lopcodes.c"
                "lua/loslib.c"
                "lua/lparser.c"
                "lua/lstate.c"
                "lua/lstring.c"
                "lua/lstrlib.c"
                "lua/ltable.c"
                "lua/ltablib.c"
                "lua/ltm.c"
                "lua/lua.c"
                "lua/luac.c"
                "lua/lundump.c"
                "lua/lvm.c"
                "lua/lzio.c"
                "lua/print.c" )

add_executable(analavi "${SOURCES}")
target_link_libraries(analavi lua)
