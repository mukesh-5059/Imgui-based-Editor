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

    // Scene Viewport Management
    std::vector<SceneViewport> sceneViewports;
    int nextSceneViewportId;
    int activeSceneViewportIndex;

    // Docking & GUI Helpers
    virtual void renderMainMenuBar();
    void performanceGui();
    void renderConsoleWindow();

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
};
