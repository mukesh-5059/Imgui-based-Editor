# 🚀 Editor Framework Cheatsheet & Code Snippets

Welcome to the **Raylib + ImGui C++ Framework Cheatsheet**. This guide provides copy-paste code snippets and mini-examples for every feature, camera controller, viewport utility, and API method.

---

## ⚡ 1. Minimal Application Template

Inherit from `Application` and override the virtual lifecycle hooks:

```cpp
#include "Application.hpp"
#include "raylib/raylib.h"

class MyEditor : public Application {
protected:
    void Init() override {
        // Setup viewports & cameras here
    }
    void Update(float dt) override {
        // Per-frame game/editor logic
    }
    void Shutdown() override {
        // Clean up resources
    }
public:
    MyEditor() : Application(1280, 720, "My Editor") {}
};

int main() {
    MyEditor app;
    app.Run();
    return 0;
}
```

---

## 🎥 2. Camera System Cheatsheet

### 2.1 3D Orbit Camera (Model Inspector)
Orbits around a target position (default `{0,0,0}`). Controls: Left/Right Click Drag to Orbit, Scroll Wheel to Zoom.

```cpp
#include "CameraController.hpp"

OrbitCamera3D orbitCam;

// 1. Initialize in Init()
orbitCam.Init({ 0.0f, 0.0f, 0.0f }, /*distance=*/ 8.0f);

// 2. Render inside Scene Viewport
AddSceneViewport("3D Model View", 
    [this]() {
        BeginMode3D(orbitCam.camera);
            DrawCube({ 0, 0, 0 }, 2, 2, 2, RED);
            DrawGrid(10, 1.0f);
        EndMode3D();
    },
    [this](SceneViewport& svp, float dt) {
        orbitCam.Update(svp, dt); // Handle mouse drag & scroll
    }
);

// 3. Render ImGui Controls in Inspector
void onInspectorGui() override {
    orbitCam.onGui("3D Orbit Camera Controls");
}
```

### 2.2 3D FreeFly Camera (WASD / FPS Navigation)
First-person flying camera. Controls: Left/Right Click Drag to Look, `WASD` to Move, `Space` = Up, `Left Shift` = Down, `Left Control` = 2x Speed.

```cpp
FreeFlyCamera3D freeFlyCam;

// 1. Initialize in Init()
freeFlyCam.Init({ 0.0f, 5.0f, 10.0f });

// 2. Render inside Scene Viewport
AddSceneViewport("FreeFly View",
    [this]() {
        BeginMode3D(freeFlyCam.camera);
            DrawGrid(14, 1.0f);
        EndMode3D();
    },
    [this](SceneViewport& svp, float dt) {
        freeFlyCam.Update(svp, dt);
    }
);

// 3. Render ImGui Controls in Inspector
void onInspectorGui() override {
    freeFlyCam.onGui("FreeFly Camera Controls");
}
```

### 2.3 2D Top-Down Camera (Panning & Zooming)
Top-down 2D camera. Controls: Left/Middle Click Drag to Pan, Scroll Wheel to Zoom.

```cpp
TopDownCamera2D topDownCam;

// 1. Initialize in Init()
topDownCam.Init({ 0.0f, 0.0f }, /*zoom=*/ 1.0f);

// 2. Render inside Scene Viewport
AddSceneViewport("2D Map View",
    [this]() {
        BeginMode2D(topDownCam.camera);
            DrawCircle(0, 0, 50.0f, BLUE);
        EndMode2D();
    },
    [this](SceneViewport& svp, float dt) {
        topDownCam.Update(svp, dt);
    }
);

// 3. Render ImGui Controls in Inspector
void onInspectorGui() override {
    topDownCam.onGui("Top-Down 2D Camera Controls");
}
```

---

## 🖱️ 3. Viewport Utilities & Mouse Raycasting

### 3.1 3D Mouse Raycasting into World Space
Cast a ray from camera through mouse cursor in viewport to select objects in 3D:

```cpp
AddSceneViewport("Raycast View",
    [this]() { /* Draw 3D scene */ },
    [this](SceneViewport& svp, float dt) {
        if (IsViewportClicked(svp, MOUSE_BUTTON_LEFT)) {
            Ray ray = GetViewportMouseRay(svp, orbitCam.camera);
            RayCollision hit = GetRayCollisionBox(ray, BoundingBox{ {-1,-1,-1}, {1,1,1} });

            if (hit.hit) {
                ConsoleLog::Get().AddLog(LogLevel::Info, "Object clicked at distance: %.2f", hit.distance);
            }
        }
    }
);
```

### 3.2 Viewport Mouse Coordinates & Click Queries

```cpp
// 1. Get Normalized Mouse Coordinates (0.0 to 1.0)
Vector2 normPos = GetViewportMousePosNorm(svp);

// 2. Get Local Pixel Coordinates inside Viewport
Vector2 localPos = GetViewportMousePosLocal(svp);

// 3. Get Mouse Coordinates at the instant Clicked
Vector2 clickNorm = GetViewportClickPosNorm(svp);
Vector2 clickLocal = GetViewportClickPosLocal(svp);

// 4. Query Single Click & Double Click Events
if (IsViewportClicked(svp, MOUSE_BUTTON_LEFT)) {
    // Single left click inside viewport
}
if (IsViewportDoubleClicked(svp, MOUSE_BUTTON_LEFT)) {
    // Double left click inside viewport
}
```

---

## 🎨 4. Texture Viewports & Hot-Reloading

### 4.1 Viewport for Image on Disk
Inspect image files from disk. Press **`Ctrl + R`** while hovering to hot-reload image:

```cpp
// Load texture viewport from disk path
int vpId = AddTextureViewport("assets/textures/wood.png");
```

### 4.2 Procedural Texture Viewport with Reload Callback
Register a dynamic Raylib texture with a reload callback (`Ctrl + R` triggers reload callback):

```cpp
Texture2D noiseTex = GenerateNoiseTexture();

AddTextureViewport(
    noiseTex,
    "Noise Preview",
    [this]() -> Texture2D {
        // Callback invoked on Ctrl + R or manual reload
        return GenerateNoiseTexture();
    },
    /*ownsTexture=*/ true,
    /*canClose=*/ true
);
```

---

## 📁 5. Custom ImGui FilePicker Modal

Open non-blocking custom ImGui file selection modal:

```cpp
#include "FilePicker.hpp"

// Open FilePicker modal dialog
FilePicker::Get().Open(
    "Import Texture Image",
    "assets/",                                    // Initial directory
    { ".png", ".jpg", ".jpeg", ".bmp", ".tga" },  // Filter extensions
    [](const std::string& selectedFilePath) {     // Callback on selection
        ConsoleLog::Get().AddLog(LogLevel::Info, "Selected file: %s", selectedFilePath.c_str());
    }
);
```

---

## 📝 6. Thread-Safe Console Logging

Log messages from anywhere in your C++ code. Raylib engine logs are automatically captured:

```cpp
#include "ConsoleLog.hpp"

// Log at different severity levels
ConsoleLog::Get().AddLog(LogLevel::Info, "Asset '%s' loaded successfully.", path);
ConsoleLog::Get().AddLog(LogLevel::Warning, "Framerate drop detected: %.1f FPS", fps);
ConsoleLog::Get().AddLog(LogLevel::Error, "Failed to compile shader '%s'", shaderName);

// Clear console buffer
ConsoleLog::Get().Clear();
```

---

## 🪟 7. Virtual GUI Hooks (`onInspectorGui` & `onCustomGui`)

### 7.1 `onInspectorGui()` - Inspector Pane
Renders controls inside the dockable **Inspector** window (`Windows -> Inspector` in menu bar):

```cpp
void onInspectorGui() override {
    ImGui::Text("Light Properties");
    ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 5.0f);
    ImGui::ColorEdit3("Light Color", (float*)&lightColor);

    // Call camera onGui helpers
    orbitCam.onGui("Camera Controls");
}
```

### 7.2 `onCustomGui()` - User ImGui Windows
Renders custom ImGui windows during the main UI pass:

```cpp
bool showToolsPanel = true;

void onCustomGui() override {
    if (showToolsPanel) {
        if (ImGui::Begin("Custom Tools Panel", &showToolsPanel)) {
            if (ImGui::Button("Spawn Cube")) {
                SpawnCube();
            }
        }
        ImGui::End();
    }
}
```

---

## ⚙️ 8. Window Toggles & Settings

Toggle built-in editor windows programmatically:

```cpp
EnableInspector(true);       // Show/Hide Inspector window
EnableConsole(true);         // Show/Hide Console Log window
EnablePerformanceGui(true);   // Show/Hide Performance Profiler window
```

---

## 🛠️ 9. CMake Build Commands

```bash
# Configure & Compile All Executables
cmake -B build && cmake --build build

# Run Test Applications
./build/camera_test            # Camera System Test App (Orbit, FreeFly, TopDown)
./build/inspector_test         # Inspector & Custom GUI Test App
./build/sceneviewport_test     # Scene Viewports Test App
./build/textureviewport_test   # Texture Viewports & FilePicker Test App
```
