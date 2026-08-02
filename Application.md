# Application Class Architecture & API Documentation

This document provides a comprehensive technical analysis of the core `Application` class (`Application.hpp`, `Application.cpp`, `Editor.cpp`) in the WorldBuilder editor engine.

---

## 1. Overview & Architectural Role

The `Application` class serves as the foundational framework and application lifecycle manager for the editor. Built on top of **Raylib** for rendering/window management and **Dear ImGui** (via `rlImGui`) for docking UI layouts, `Application` abstracts away:
1. **Windowing & Main Loop**: Execution lifecycle management, target framerate controls, and frame time tracking.
2. **Offscreen Framebuffer Rendering**: Multi-viewport rendering using Raylib's `RenderTexture2D`.
3. **ImGui Docking Layout System**: Dynamic splitting for Main Viewports, Inspector panels, Console log, and Table visualizers.
4. **Viewport Management APIs**: Supporting both **Scene Viewports** (3D/2D render targets with custom drawing and picking callbacks) and **Texture Viewports** (disk/memory dynamic texture previewers).

---

## 2. Helper Data Structures

Before detailing `Application` members, two key supporting structures manage viewport state within the class.

### 2.1 `TextureViewport`
A workspace tab designed for inspecting and reloading static or dynamic 2D textures.

| Field | Type | Description |
| :--- | :--- | :--- |
| `id` | `int` | Unique identifier assigned when created. Defaults to `0`. |
| `name` | `std::string` | Label displayed on the ImGui tab header. |
| `filePath` | `char[512]` | Fixed buffer storing file path for images loaded from disk. |
| `loadedPath` | `std::string` | Descriptive source path or callback identifier string. |
| `texture` | `Texture2D` | Raylib GPU texture handle. |
| `isLoaded` | `bool` | Flags whether a valid GPU texture is currently allocated. |
| `open` | `bool` | ImGui tab visibility / open state. |
| `canClose` | `bool` | Specifies whether user can close this tab. |
| `ownsTexture` | `bool` | When `true`, `Application` is responsible for calling `UnloadTexture()`. |
| `reloadCallback` | `std::function<Texture2D()>` | Optional callback function executed to dynamically refresh the texture. |

---

### 2.2 `SceneViewport`
A workspace tab representing a dedicated offscreen render target framebuffer.

| Field | Type | Description |
| :--- | :--- | :--- |
| `id` | `int` | Unique identifier assigned when created. Defaults to `0`. |
| `name` | `std::string` | Label displayed on the ImGui tab header. |
| `renderTexture` | `RenderTexture2D` | Raylib offscreen render target containing color & depth buffers. |
| `isLoaded` | `bool` | Flags whether the render texture is initialized. |
| `open` | `bool` | ImGui tab visibility / open state. |
| `canClose` | `bool` | Specifies whether user can close this tab. |
| `userData` | `void*` | Arbitrary user context data attached to this viewport instance. |
| `sceneDrawCallback` | `std::function<void(SceneViewport&)>` | Custom scene rendering function executed inside `BeginTextureMode()`. |
| `inspectorDrawCallback` | `std::function<void(SceneViewport&)>` | Custom inspector UI rendering function executed when tab is active. |
| `clickCallback` | `std::function<void(SceneViewport&, Vector2 normPos)>` | Event handler triggered when user left-clicks inside the viewport. |

---

## 3. Member Variables

`Application` encapsulates state for window dimensions, performance statistics, rendering resources, UI layout splitters, and viewports.

### 3.1 Core Window & Lifecycle State
* `int width`  
  Current width of the application window and base resolution for offscreen render targets.
* `int height`  
  Current height of the application window and base resolution for offscreen render targets.
* `const char* title`  
  Window title passed to Raylib `InitWindow()`.
* `bool running`  
  Main loop control flag. When set to `false`, the main execution loop terminates gracefully.

### 3.2 Performance & Timing Attributes
* `int m_targetFps`  
  Target framerate cap (e.g., 60 FPS). Modified dynamically via the Performance GUI widget.
* `float m_frameTimeHistory[100]`  
  Circular history buffer storing the last 100 frame durations in milliseconds for histogram plotting.
* `int m_lastTargetFps`  
  Cached target FPS value used to trigger Raylib `SetTargetFPS()` calls only when the setting changes.
* `int m_frameTimeIndex`  
  Current write head index into the `m_frameTimeHistory` array.

### 3.3 Framebuffer & Viewport Containers
* `RenderTexture2D sceneRenderTexture`  
  Fallback offscreen render target used when no dynamic `sceneViewports` exist.
* `std::vector<TextureViewport> textureViewports`  
  List of active texture inspection tabs.
* `int nextViewportId`  
  Monotonically increasing ID counter for texture viewports (starts at 1).
* `std::vector<SceneViewport> sceneViewports`  
  List of active scene workspace tabs.
* `int activeSceneViewportIndex`  
  Index of the currently active/focused scene viewport tab within `sceneViewports`.
* `int nextSceneViewportId`  
  Monotonically increasing ID counter for scene viewports (starts at 1).

### 3.4 Interactive Mouse & Raycasting State
* `bool is3DViewportHovered`  
  Indicates whether the mouse cursor is currently positioned within the active viewport render region.
* `Vector2 viewportMouseNorm`  
  Normalized coordinates of the mouse within the viewport rectangle, mapped to range $[0.0, 1.0] \times [0.0, 1.0]$.

### 3.5 UI Layout & Panel Toggles
* `float inspectorWidth`  
  Pixel width of the right-side Inspector panel (resizable via splitter drag).
* `float consoleHeight`  
  Pixel height of the bottom panel housing Console and Table Visualizer widgets (resizable via splitter drag).
* `bool showConsole`  
  Visibility toggle flag for the Console Log widget.
* `bool showTableVisualizer`  
  Visibility toggle flag for the Table Visualizer widget.
* `bool showInspector`  
  Visibility toggle flag for the right Inspector panel.

---

## 4. Methods Detailed Analysis

### 4.1 Constructor & Destructor

#### `Application(int width = 1280, int height = 720, const char* title = "WorldBuilder")`
* **Access**: `public`
* **Description**: Constructs the `Application` instance, initializing member variables with default window dimensions, layout dimensions, and initial visibility flags.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L6-L12)

#### `virtual ~Application()`
* **Access**: `public`
* **Description**: Safely cleans up allocated GPU resources. Iterates over `textureViewports` to call `UnloadTexture()` for owned textures, and iterates over `sceneViewports` to call `UnloadRenderTexture()`.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L14-L27)

---

### 4.2 Engine Lifecycle & Core Control

#### `void Run()`
* **Access**: `public`
* **Description**: Executes the main application lifecycle:
  1. Configures resizable window flags and detects primary monitor dimensions.
  2. Calls Raylib `InitWindow()`, toggles fullscreen, sets initial FPS limit, and initializes `rlImGui`.
  3. Registers log callbacks and initializes base `sceneRenderTexture`.
  4. Calls `Init()` for derived application initialization.
  5. Enters main loop running while `!WindowShouldClose() && running`:
     - Updates delta time and framerate history buffer.
     - Calls `Update(deltaTime)`.
     - Performs offscreen texture rendering (`BeginTextureMode`) for active scene viewports.
     - Renders ImGui layout (`editorGui()`, `performanceGui()`).
  6. On loop exit, unloads framebuffers, calls `Shutdown()`, shuts down `rlImGui`, and closes the window.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L84-L171)

#### `virtual void Init()`
* **Access**: `protected` (virtual override point)
* **Description**: Hook called once during startup inside `Run()` before entering the main execution loop. Intended to be implemented by child classes to initialize scene objects, cameras, or load resources.
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L91)

#### `virtual void Update(float deltaTime)`
* **Access**: `protected` (virtual override point)
* **Description**: Called every frame before rendering. Implement custom per-frame logic, animations, or camera movements here.
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L92)

#### `virtual void SceneDraw()`
* **Access**: `protected` (virtual override point)
* **Description**: Fallback rendering callback used when no scene viewports possess custom `sceneDrawCallback` functions. Code executed here renders into offscreen render textures.
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L93)

#### `virtual void DrawInspectorUI()`
* **Access**: `protected` (virtual override point)
* **Description**: Renders default Inspector controls when no active scene viewport provides a custom `inspectorDrawCallback`.
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L94)

#### `virtual void On3DViewportClicked(Vector2 mouseNormInViewport)`
* **Access**: `protected` (virtual override point)
* **Description**: Handler invoked when the user left-clicks inside a viewport. Receives normalized coordinates $[0..1]$ to facilitate object selection or raycasting.
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L95)

#### `virtual void Shutdown()`
* **Access**: `protected` (virtual override point)
* **Description**: Cleanup hook invoked inside `Run()` right before window closure.
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L96)

---

### 4.3 Internal GUI & Rendering Support Methods

#### `void performanceGui()`
* **Access**: `protected`
* **Description**: Draws an ImGui window displaying current FPS, frame rendering duration, target FPS slider, and a historical frame time histogram (`PlotHistogram`).
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L128-L139)

#### `void renderResolutionGui()`
* **Access**: `protected`
* **Description**: Renders header controls in the Inspector panel providing sliders for adjusting render resolution dimensions dynamically.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L141-L156)

#### `void SetRenderResolution(int newWidth, int newHeight)`
* **Access**: `protected`
* **Description**: Resizes all offscreen framebuffers (`sceneRenderTexture` and all `sceneViewports`) to match `newWidth` and `newHeight`, freeing old textures and reallocating memory.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L173-L194)

#### `void editorGui()`
* **Access**: `protected`
* **Description**: Constructs the primary docking interface:
  - Configures full-screen root container.
  - Implements tab bar for `SceneViewport` and `TextureViewport` tabs.
  - Handles mouse interaction and click callbacks on viewport items.
  - Manages horizontal and vertical resizer splitters for dynamic panel resizing.
  - Displays bottom panel with `ConsoleLog` and `TableVisualizer`.
  - Displays right Inspector panel.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L158-L501)

---

### 4.4 Texture Viewport API

#### `int AddTextureViewport(const std::string& initialPath = "")`
* **Access**: `protected`
* **Description**: Creates a new `TextureViewport` tab. If `initialPath` is supplied and valid, loads the image into GPU memory. Returns the assigned viewport ID.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L8-L26)

#### `int AddTextureViewport(Texture2D texture, const std::string& name = "", std::function<Texture2D()> reloadCallback = nullptr, bool ownsTexture = false, bool canClose = true)`
* **Access**: `protected`
* **Description**: Overloaded factory to register a texture viewport from an existing Raylib `Texture2D` handle with custom reload callbacks and ownership settings.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L28-L43)

#### `void RemoveTextureViewport(int index)`
* **Access**: `protected`
* **Description**: Frees GPU memory (if owned) and removes the viewport at `index` from `textureViewports`.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L45-L53)

#### `void SetTextureViewportCallback(int viewportId, std::function<Texture2D()> reloadCallback)`
* **Access**: `protected`
* **Description**: Associates a dynamic reload callback with the texture viewport matching `viewportId`.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L55-L62)

#### `void SetTextureViewportTexture(int viewportId, Texture2D texture, bool ownsTexture = false)`
* **Access**: `protected`
* **Description**: Replaces the active texture of a viewport with a new `Texture2D`, unloading any previously owned texture.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L64-L79)

#### `void ReloadTextureViewport(TextureViewport& vp)`
* **Access**: `protected`
* **Description**: Refreshes a `TextureViewport` either by executing its `reloadCallback` or re-reading the image file from disk.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L81-L108)

#### `void ReloadTextureViewport(int viewportId)`
* **Access**: `protected`
* **Description**: Finds viewport by `viewportId` and invokes `ReloadTextureViewport(TextureViewport&)`.
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L110-L117)

#### `TextureViewport* GetTextureViewport(int viewportId)`
* **Access**: `protected`
* **Description**: Searches `textureViewports` for a matching `viewportId` and returns a pointer to it (or `nullptr`).
* **Source**: [Editor.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Editor.cpp#L119-L126)

---

### 4.5 Scene Viewport API

#### `int AddSceneViewport(...)`
* **Access**: `protected`
* **Signature**:
  ```cpp
  int AddSceneViewport(const std::string& name = "Scene Viewport",
                       std::function<void(SceneViewport&)> sceneDraw = nullptr,
                       std::function<void(SceneViewport&)> inspectorDraw = nullptr,
                       std::function<void(SceneViewport&, Vector2 normPos)> onClick = nullptr,
                       bool canClose = true,
                       void* userData = nullptr);
  ```
* **Description**: Registers a new scene viewport workspace tab with its own offscreen `RenderTexture2D` and event callbacks.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L29-L53)

#### `void RemoveSceneViewport(int index)`
* **Access**: `protected`
* **Description**: Unloads the render target associated with the scene viewport at `index` and removes it from `sceneViewports`, updating active selection indices.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L55-L68)

#### `SceneViewport* GetActiveSceneViewport()`
* **Access**: `protected`
* **Description**: Returns a pointer to the currently selected `SceneViewport` (or `nullptr` if none active).
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L70-L75)

#### `SceneViewport* GetSceneViewport(int viewportId)`
* **Access**: `protected`
* **Description**: Returns a pointer to the `SceneViewport` matching `viewportId`.
* **Source**: [Application.cpp](file:///home/mukes/dev/C++/Projects/editor/src/core/Application.cpp#L77-L82)

---

### 4.6 UI Visibility Toggles

#### `void EnableConsole(bool enable)`
* **Access**: `public`
* **Description**: Toggles visibility of the console log window (`showConsole = enable`).
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L104)

#### `void EnableTableVisualizer(bool enable)`
* **Access**: `public`
* **Description**: Toggles visibility of the table visualizer window (`showTableVisualizer = enable`).
* **Source**: [Application.hpp](file:///home/mukes/dev/C++/Projects/editor/src/includes/Application.hpp#L105)

#### `void EnableInspector(bool enable)`
* **Access**: `public`
* **Description**: Toggles visibility of the inspector panel (`showInspector = enable`).
* **Source**: [Application.hpp](file:///home/mukes/dev/C:///Projects/editor/src/includes/Application.hpp#L106)
