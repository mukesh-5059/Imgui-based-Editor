#pragma once
#include "raylib/raylib.h"
#include "ConsoleLog.hpp"
#include "utils.hpp"
#include <vector>
#include <string>

class Application {
protected:
    int width;
    int height;
    const char* title;
    bool running;

    int m_targetFps;
    float m_frameTimeHistory[100];
    int m_lastTargetFps;
    int m_frameTimeIndex;

    bool showConsole;
    bool showPerformance;
    bool showInspector;
    bool fullscreen;

    // Scene Viewport Management
    std::vector<SceneViewport> sceneViewports;
    int nextSceneViewportId;
    int activeSceneViewportIndex;

    // Texture Viewport Management
    std::vector<TextureViewport> textureViewports;
    int nextViewportId;

    // Docking & GUI Helpers
    virtual void renderMainMenuBar();
    void performanceGui();
    void renderConsoleWindow();
    void renderInspectorWindow();

    // Virtual GUI Hooks for Custom Derived Applications
    virtual void onInspectorGui() {}
    virtual void onCustomGui() {}

    // Texture Viewport Factory & API
    int AddTextureViewport(const std::string& initialPath = "");
    int AddTextureViewport(Texture2D texture, const std::string& name = "", std::function<Texture2D()> reloadCallback = nullptr, bool ownsTexture = false, bool canClose = true);
    void RemoveTextureViewport(int index);
    void SetTextureViewportCallback(int viewportId, std::function<Texture2D()> reloadCallback);
    void SetTextureViewportTexture(int viewportId, Texture2D texture, bool ownsTexture = false);
    void ReloadTextureViewport(int viewportId);
    void ReloadTextureViewport(TextureViewport& vp);
    TextureViewport* GetTextureViewport(int viewportId);
    void textureViewportInputs(float dt);

    // Scene Viewport Factory & API
    int AddSceneViewport(const std::string& name = "Scene Viewport",
                         std::function<void()> drawCallback = nullptr,
                         std::function<void(SceneViewport& svp, float dt)> inputCallback = nullptr,
                         std::function<void(SceneViewport&, Vector2 normPos)> onClick = nullptr,
                         bool canClose = true,
                         void* userData = nullptr);
    void RemoveSceneViewport(int index);
    SceneViewport* GetActiveSceneViewport();
    SceneViewport* GetSceneViewport(int viewportId);
    void sceneViewportInputs(float dt);
    void sceneViewportsRender();

    virtual void Init() {}
    virtual void Update(float deltaTime) {}
    virtual void Shutdown() {}

public:
    Application(int width = 1280, int height = 720, const char* title = "WorldBuilder");
    virtual ~Application();

    void Run();

    void EnableConsole(bool enable) { showConsole = enable; }
    void EnablePerformanceGui(bool enable) { showPerformance = enable; }
    void EnableInspector(bool enable) { showInspector = enable; }

    void EnableFullscreen(bool enable = true);
    void SetFullscreen(bool enable);
    void DisableFullscreen() { EnableFullscreen(false); }
    bool IsFullscreen() const;
};
