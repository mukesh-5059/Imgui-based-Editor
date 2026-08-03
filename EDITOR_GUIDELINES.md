# Editor Framework Architecture & Developer Guidelines

Welcome to the **WorldBuilder Editor Framework**. This guide provides complete technical guidelines on how to inherit, configure, and extend the engine's core `Application` architecture.

---

## 1. Architectural Overview

The framework is built on **Raylib** for 3D/2D rendering and **Dear ImGui (Docking Branch)** for window management and docking layouts.


### Core Features
- **Polymorphic Application Class**: Derive from [`Application`](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L8) to create custom editors, games, or visualization tools.
- **Dynamic Scene Viewports**: Create isolated 3D or 2D render targets with independent draw callbacks and input handlers.
- **Native ImGui Docking**: Complete docking, tabbing, and splitting support automatically saved to `imgui.ini`.
- **Render Optimization**: Offscreen scene framebuffers render **only when visible, hovered, or active**, consuming zero GPU cycles when hidden.
- **Thread-Safe Console Logging**: Automatic Raylib trace log redirection with `ConsoleLog` filtering and time-stamping.

---

## 2. Quick Start: Creating a Custom Application

To build an application using the framework, inherit from `Application` and override the virtual lifecycle hooks:

```cpp
#include "Application.hpp"
#include "raylib/raylib.h"

class MyEditorApp : public Application {
private:
    Camera3D camera3D = { 0 };

protected:
    void Init() override {
        // 1. Setup Camera
        camera3D.position = (Vector3){ 0.0f, 6.0f, 10.0f };
        camera3D.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
        camera3D.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
        camera3D.fovy     = 45.0f;
        camera3D.projection = CAMERA_PERSPECTIVE;

        // 2. Register a 3D Scene Viewport
        AddSceneViewport(
            "3D Viewport",
            // Draw Callback: Executes inside BeginTextureMode
            [this]() {
                BeginMode3D(camera3D);
                    DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                    DrawGrid(10, 1.0f);
                EndMode3D();
            },
            // Input Callback: Executes when viewport is hovered or actively dragged
            [this](SceneViewport& svp, float dt) {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    // Update camera based on mouse movement
                }
            }
        );
    }

    void Update(float deltaTime) override {
        // Per-frame application logic updates here
    }

    void Shutdown() override {
        // Cleanup resources here before window closure
    }

public:
    MyEditorApp() : Application(1280, 720, "My Custom Editor") {}
};

int main() {
    MyEditorApp app;
    app.Run();
    return 0;
}
```

---

## 3. Working with Scene Viewports

The framework manages viewports using the [`SceneViewport`](file:///home/mukes/dev/C++/Projects/editor/src/includes/utils.hpp#L7) structure defined in [`utils.hpp`](file:///home/mukes/dev/C++/Projects/editor/src/includes/utils.hpp).

### 3.1 Registering a Viewport
Use `AddSceneViewport` to register a new viewport tab:

```cpp
int AddSceneViewport(
    const std::string& name = "Scene Viewport",
    std::function<void()> drawCallback = nullptr,
    std::function<void(SceneViewport& svp, float dt)> inputCallback = nullptr,
    std::function<void(SceneViewport&, Vector2 normPos)> onClick = nullptr,
    bool canClose = true,
    void* userData = nullptr
);
```

### 3.2 Viewport State Fields
Inside your callbacks, access the following `SceneViewport&` state fields:

| Field | Type | Description |
| :--- | :--- | :--- |
| `name` | `std::string` | Title displayed on the ImGui window/tab header. |
| `isVisible` | `bool` | `true` if the ImGui tab is currently active & visible on screen. |
| `isHovered` | `bool` | `true` if the mouse cursor is physically inside the viewport bounds. |
| `isActive` | `bool` | `true` while the mouse is held down after clicking inside the viewport. |
| `mousePosNorm` | `Vector2` | Normalized mouse coordinates $(u, v \in [0.0, 1.0])$ within the viewport. |
| `mousePosLocal` | `Vector2` | Local pixel mouse coordinates $(x, y)$ relative to viewport top-left. |
| `userData` | `void*` | Pointer to arbitrary context data attached to this viewport. |

---

## 4. Input Handling Guidelines

Input dispatching is **isolated per viewport**:

1. **Automatic Hover & Active Drag Filtering**: The framework checks `(isHovered || isActive)` internally before invoking your `inputCallback`. You do **not** need to manually write `if (isHovered)` checks inside your callbacks.
2. **Dragging Outside Bounds**: If the user clicks down inside a viewport and drags the mouse outside the window, `isActive` stays `true`, ensuring mouse rotation or dragging continues uninterrupted until the button is released.

```cpp
[this](SceneViewport& svp, float dt) {
    // Guaranteed to execute ONLY when this viewport is hovered or actively dragged!
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        // Rotate 3D camera
    }
}
```

---

## 5. Console Logging Guidelines

The framework includes a thread-safe logging system [`ConsoleLog`](file:///home/mukes/dev/C++/Projects/editor/src/includes/ConsoleLog.hpp#L21).

### 5.1 Adding Custom Logs
Log messages from anywhere in your codebase using `ConsoleLog::Get()`:

```cpp
#include "ConsoleLog.hpp"

// Info Log
ConsoleLog::Get().AddLog(LogLevel::Info, "Loaded asset '%s' successfully.", assetPath);

// Warning Log
ConsoleLog::Get().AddLog(LogLevel::Warning, "High memory usage detected: %d MB.", memUsage);

// Error Log
ConsoleLog::Get().AddLog(LogLevel::Error, "Failed to load shader file '%s'.", shaderPath);
```

### 5.2 Raylib System Log Redirection
Raylib's internal engine messages (`INFO`, `WARNING`, `ERROR`) are automatically redirected to `ConsoleLog` via `SetTraceLogCallback(RaylibTraceLogCallback)` during application startup.

---

## 6. Build & Compilation Instructions

Add your executable target to [`CMakeLists.txt`](file:///home/mukes/dev/C++/Projects/editor/CMakeLists.txt):

```cmake
file(GLOB my_app_src src/apps/MyApp/*.cpp)
add_executable(my_app ${imgui} ${rlimgui} ${core_src} ${my_app_src})
target_include_directories(my_app PRIVATE ${SHARED_INCLUDES})
target_link_libraries(my_app PRIVATE ${SHARED_LIBS})
```

Build and execute via terminal:

```bash
# Configure & compile
cmake -B build && cmake --build build

# Run application
./build/my_app
```
