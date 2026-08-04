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
    dependencies/includes/imgui
    dependencies/includes/rlimgui
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

# 1. Scene Viewport Testing Executable Target
file(GLOB scene_viewport_test_src src/apps/SceneViewportTesting/*.cpp)
add_executable(sceneviewport_test ${imgui} ${rlimgui} ${core_src} ${scene_viewport_test_src})
target_include_directories(sceneviewport_test PRIVATE ${SHARED_INCLUDES})
target_link_libraries(sceneviewport_test PRIVATE ${SHARED_LIBS})

# 2. Texture Viewport Testing Executable Target
file(GLOB texture_viewport_test_src src/apps/TextureViewportTesting/*.cpp)
add_executable(textureviewport_test ${imgui} ${rlimgui} ${core_src} ${texture_viewport_test_src})
target_include_directories(textureviewport_test PRIVATE ${SHARED_INCLUDES})
target_link_libraries(textureviewport_test PRIVATE ${SHARED_LIBS})

# 3. Inspector & Custom GUI Testing Executable Target
file(GLOB inspector_test_src src/apps/InspectorTesting/*.cpp)
add_executable(inspector_test ${imgui} ${rlimgui} ${core_src} ${inspector_test_src})
target_include_directories(inspector_test PRIVATE ${SHARED_INCLUDES})
target_link_libraries(inspector_test PRIVATE ${SHARED_LIBS})

# 4. Camera Controllers Testing Executable Target
file(GLOB camera_test_src src/apps/CameraTesting/*.cpp)
add_executable(camera_test ${imgui} ${rlimgui} ${core_src} ${camera_test_src})
target_include_directories(camera_test PRIVATE ${SHARED_INCLUDES})
target_link_libraries(camera_test PRIVATE ${SHARED_LIBS})

# 5. Standalone Docking Test Executable Target
file(GLOB docking_test_src src/apps/DockingTest/*.cpp)
add_executable(docking_test ${imgui} ${rlimgui} ${docking_test_src})
target_include_directories(docking_test PRIVATE ${SHARED_INCLUDES})
target_link_libraries(docking_test PRIVATE ${SHARED_LIBS})