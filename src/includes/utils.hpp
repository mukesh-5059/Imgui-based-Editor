#pragma once
#include "raylib/raylib.h"
#include <string>
#include <functional>

// Scene Viewport Structure (Offscreen Render Target Framebuffer)
struct SceneViewport {
    int id = 0;
    std::string name;
    RenderTexture2D renderTexture = { 0 };
    bool isLoaded = false;
    bool hasDrawnOnce = false;
    bool open = true;
    bool canClose = true;

    // Visibility, Focus & Input State
    bool isVisible = false;  // True if ImGui tab is visible on screen!
    bool isHovered = false;  // True if mouse is hovering over the viewport
    bool isActive = false;   // True if user is actively dragging/holding mouse inside viewport

    Vector2 mousePosNorm = { 0.0f, 0.0f };   // Normalized (0.0 to 1.0)
    Vector2 mousePosLocal = { 0.0f, 0.0f };  // Local Pixel Coordinates inside viewport

    void* userData = nullptr;

    // Callbacks
    std::function<void()> drawCallback = nullptr;
    std::function<void(SceneViewport& svp, float dt)> inputCallback = nullptr;
    std::function<void(SceneViewport&, Vector2 normPos)> clickCallback = nullptr;
};

// Texture Viewport Structure (Disk / Memory GPU Texture Preview & Callbacks)
struct TextureViewport {
    int id = 0;
    std::string name;
    char filePath[512] = "";
    std::string loadedPath;
    Texture2D texture = { 0 };
    bool isLoaded = false;
    bool open = true;
    bool canClose = true;
    bool ownsTexture = true;
    bool pendingReload = false; // Set to true by async file picker thread to trigger GPU texture reload on main thread

    bool isVisible = false;
    bool isHovered = false;
    bool isActive = false;

    // Callback function invoked to refresh / reload GPU texture
    std::function<Texture2D()> reloadCallback = nullptr;
};
