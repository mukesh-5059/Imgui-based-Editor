cmake_minimum_required(VERSION 3.10.0)
project(MocDa1 VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Common sources (Core UI, ImGui, rlImGui wrapper)
file(GLOB core_src src/core/*.cpp)
file(GLOB imgui dependencies/lib/imgui/*.cpp)
file(GLOB rlimgui dependencies/lib/rlimgui/*.cpp)

# Shared include directories
set(SHARED_INCLUDES
    dependencies/includes
    src/includes
)

# Shared libraries
set(SHARED_LIBS
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/raylib/libraylib.a"
    GL
    m
    pthread
    dl
    rt
    X11
)

#Common testing
file(GLOB testing src/apps/Testing/*.cpp)
add_executable(testing ${imgui} ${rlimgui} ${core_src} ${testing})
target_include_directories(testing PRIVATE ${SHARED_INCLUDES})
target_link_libraries(testing PRIVATE ${SHARED_LIBS})