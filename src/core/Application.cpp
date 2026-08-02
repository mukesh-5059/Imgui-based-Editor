#include "Application.hpp"
#include "ConsoleLog.hpp"
#include "raylib/raylib.h"
#include "rlimgui/rlImGui.h"

Application::Application(int width, int height, const char* title)
    : width(width), height(height), title(title), running(true),
      nextViewportId(1), activeSceneViewportIndex(0), nextSceneViewportId(1),
      is3DViewportHovered(false), viewportMouseNorm({ 0.0f, 0.0f }),
      inspectorWidth(340.0f), consoleHeight(200.0f),
      showConsole(true), showTableVisualizer(false), showInspector(true) {
}

Application::~Application() {
    for (auto& vp : textureViewports) {
        if (vp.isLoaded && vp.ownsTexture && vp.texture.id > 0) {
            UnloadTexture(vp.texture);
            vp.isLoaded = false;
        }
    }
    for (auto& svp : sceneViewports) {
        if (svp.isLoaded && svp.renderTexture.id > 0) {
            UnloadRenderTexture(svp.renderTexture);
            svp.isLoaded = false;
        }
    }
}

int Application::AddSceneViewport(const std::string& name,
                                  std::function<void(SceneViewport&)> sceneDraw,
                                  std::function<void(SceneViewport&)> inspectorDraw,
                                  std::function<void(SceneViewport&, Vector2 normPos)> onClick,
                                  bool canClose,
                                  void* userData) {
    SceneViewport svp;
    svp.id = nextSceneViewportId++;
    svp.name = name.empty() ? ("Viewport " + std::to_string(svp.id)) : name;
    svp.canClose = canClose;
    svp.userData = userData;
    svp.sceneDrawCallback = sceneDraw;
    svp.inspectorDrawCallback = inspectorDraw;
    svp.clickCallback = onClick;

    if (width > 0 && height > 0) {
        svp.renderTexture = LoadRenderTexture(width, height);
        SetTextureFilter(svp.renderTexture.texture, TEXTURE_FILTER_BILINEAR);
        svp.isLoaded = true;
    }

    sceneViewports.push_back(svp);
    activeSceneViewportIndex = (int)sceneViewports.size() - 1;
    return svp.id;
}

void Application::RemoveSceneViewport(int index) {
    if (index >= 0 && index < (int)sceneViewports.size()) {
        if (sceneViewports[index].isLoaded && sceneViewports[index].renderTexture.id > 0) {
            UnloadRenderTexture(sceneViewports[index].renderTexture);
        }
        sceneViewports[index].isLoaded = false;
        sceneViewports.erase(sceneViewports.begin() + index);

        if (activeSceneViewportIndex >= (int)sceneViewports.size()) {
            activeSceneViewportIndex = (int)sceneViewports.size() - 1;
        }
        if (activeSceneViewportIndex < 0) activeSceneViewportIndex = 0;
    }
}

SceneViewport* Application::GetActiveSceneViewport() {
    if (activeSceneViewportIndex >= 0 && activeSceneViewportIndex < (int)sceneViewports.size()) {
        return &sceneViewports[activeSceneViewportIndex];
    }
    return nullptr;
}

SceneViewport* Application::GetSceneViewport(int viewportId) {
    for (auto& svp : sceneViewports) {
        if (svp.id == viewportId) return &svp;
    }
    return nullptr;
}

void Application::Run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    int monitor = GetCurrentMonitor();
    int mWidth = GetMonitorWidth(monitor);
    int mHeight = GetMonitorHeight(monitor);
    if (mWidth > 0 && mHeight > 0) {
        width = mWidth;
        height = mHeight;
    }

    InitWindow(width, height, title);
    ToggleFullscreen();

    SetTargetFPS(60);
    rlImGuiSetup(true);

    SetTraceLogCallback(RaylibTraceLogCallback);
    ConsoleLog::Get().AddLog(LogLevel::Info, "WorldBuilder Editor Console initialized.");

    sceneRenderTexture = LoadRenderTexture(width, height);
    SetTextureFilter(sceneRenderTexture.texture, TEXTURE_FILTER_BILINEAR);

    Init();
    m_targetFps = 60;
    m_lastTargetFps = 60;
    for (int i = 0; i < 100; ++i) m_frameTimeHistory[i] = 0.0f;
    m_frameTimeIndex = 0;

    while (!WindowShouldClose() && running) {
        float deltaTime = GetFrameTime();

        if (m_targetFps != m_lastTargetFps) {
            SetTargetFPS(m_targetFps);
            m_lastTargetFps = m_targetFps;
        }

        m_frameTimeHistory[m_frameTimeIndex] = deltaTime * 1000.0f;
        m_frameTimeIndex = (m_frameTimeIndex + 1) % 100;
        
        Update(deltaTime);

        // 1. Offscreen Scene Rendering to Textures
        if (sceneViewports.empty()) {
            BeginTextureMode(sceneRenderTexture);
                ClearBackground(DARKGRAY);
                SceneDraw();
            EndTextureMode();
        } else {
            for (size_t i = 0; i < sceneViewports.size(); ++i) {
                auto& svp = sceneViewports[i];
                if (!svp.isLoaded || svp.renderTexture.id == 0) {
                    svp.renderTexture = LoadRenderTexture(width, height);
                    SetTextureFilter(svp.renderTexture.texture, TEXTURE_FILTER_BILINEAR);
                    svp.isLoaded = true;
                }
                BeginTextureMode(svp.renderTexture);
                    ClearBackground(DARKGRAY);
                    if (svp.sceneDrawCallback) {
                        svp.sceneDrawCallback(svp);
                    } else {
                        SceneDraw();
                    }
                EndTextureMode();
            }
        }

        // 2. Pure ImGui-Driven Fullscreen Editor Layout
        BeginDrawing();
            ClearBackground(BLACK);

            rlImGuiBegin();

            // Render Full Editor GUI
            editorGui();

            // Floating Performance GUI Window
            performanceGui();

            rlImGuiEnd();

        EndDrawing();
    }

    UnloadRenderTexture(sceneRenderTexture);
    Shutdown();
    rlImGuiShutdown();
    CloseWindow();
}

void Application::SetRenderResolution(int newWidth, int newHeight) {
    if (newWidth < 256) newWidth = 256;
    if (newHeight < 144) newHeight = 144;

    if (sceneRenderTexture.texture.width != newWidth || sceneRenderTexture.texture.height != newHeight) {
        UnloadRenderTexture(sceneRenderTexture);
        sceneRenderTexture = LoadRenderTexture(newWidth, newHeight);
        SetTextureFilter(sceneRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
        
        for (auto& svp : sceneViewports) {
            if (svp.isLoaded && svp.renderTexture.id > 0) {
                UnloadRenderTexture(svp.renderTexture);
            }
            svp.renderTexture = LoadRenderTexture(newWidth, newHeight);
            SetTextureFilter(svp.renderTexture.texture, TEXTURE_FILTER_BILINEAR);
            svp.isLoaded = true;
        }

        width = newWidth;
        height = newHeight;
    }
}