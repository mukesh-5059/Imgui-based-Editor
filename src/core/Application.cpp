#include "Application.hpp"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "FilePicker.hpp"

Application::Application(int width, int height, const char* title)
    : width(width), height(height), title(title), running(true), fullscreen(true),
      m_targetFps(60), m_lastTargetFps(60), m_frameTimeIndex(0),
      showConsole(true), showPerformance(false), showInspector(true),
      nextSceneViewportId(1), activeSceneViewportIndex(0), nextViewportId(1) {
    for (int i = 0; i < 100; ++i) {
        m_frameTimeHistory[i] = 0.0f;
    }
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

void Application::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit Application")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Inspector", nullptr, &showInspector);
            ImGui::MenuItem("Console Log", nullptr, &showConsole);
            ImGui::MenuItem("Performance Profiler", nullptr, &showPerformance);
            bool fs = IsFullscreen();
            if (ImGui::MenuItem("Fullscreen", "F11", &fs)) {
                SetFullscreen(fs);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Application::performanceGui() {
    if (!showPerformance) return;

    if (ImGui::Begin("Performance", &showPerformance)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms/frame ", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::SliderInt("Target FPS", &m_targetFps, 30, 300);
        ImGui::Separator();
        ImGui::PlotHistogram("Frame Times", m_frameTimeHistory, 100, m_frameTimeIndex, nullptr, 0.0f, 33.3f, ImVec2(0, 80));
    }
    ImGui::End();
}

void Application::renderConsoleWindow() {
    if (!showConsole) return;

    if (ImGui::Begin("Console Log", &showConsole)) {
        ConsoleLog::Get().Draw("Console Log");
    }
    ImGui::End();
}

void Application::renderInspectorWindow() {
    if (!showInspector) return;

    if (ImGui::Begin("Inspector", &showInspector)) {
        onInspectorGui();
    }
    ImGui::End();
}

void Application::Run() {
    unsigned int flags = FLAG_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= FLAG_FULLSCREEN_MODE;
    }
    SetConfigFlags(flags);
    InitWindow(width, height, title);
    SetTargetFPS(60);

    rlImGuiSetup(true);

    // Enable Dear ImGui Native Docking
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Register Raylib Trace Log Callback & Add Initial Console Entry
    SetTraceLogCallback(RaylibTraceLogCallback);
    ConsoleLog::Get().AddLog(LogLevel::Info, "%s Console Log initialized.", title);

    Init();

    while (!WindowShouldClose() && running) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_F11)) {
            SetFullscreen(!IsFullscreen());
        }

        if (m_targetFps != m_lastTargetFps) {
            SetTargetFPS(m_targetFps);
            m_lastTargetFps = m_targetFps;
        }

        m_frameTimeHistory[m_frameTimeIndex] = deltaTime * 1000.0f;
        m_frameTimeIndex = (m_frameTimeIndex + 1) % 100;

        Update(deltaTime);

        // 1. Offscreen Scene Pass
        sceneViewportsRender();

        // 2. Main Screen Rendering & ImGui DockSpace Pass
        BeginDrawing();
            ClearBackground(BLACK);

            rlImGuiBegin();

            renderMainMenuBar();

            // Submit Central DockSpace over Main Viewport
            ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspaceFlags);

            sceneViewportInputs(deltaTime);
            textureViewportInputs(deltaTime);
            renderConsoleWindow();
            renderInspectorWindow();
            performanceGui();

            // Call Virtual Custom GUI Hook for User-Defined Windows
            onCustomGui();

            // Render Custom ImGui File Picker Modal Dialog
            FilePicker::Get().Draw();

            rlImGuiEnd();

        EndDrawing();
    }

    Shutdown();
    rlImGuiShutdown();
    CloseWindow();
}

void Application::SetFullscreen(bool enable) {
    fullscreen = enable;
    if (IsWindowReady()) {
        if (IsWindowFullscreen() != enable) {
            ToggleFullscreen();
        }
    }
}

void Application::EnableFullscreen(bool enable) {
    SetFullscreen(enable);
}

bool Application::IsFullscreen() const {
    if (IsWindowReady()) {
        return IsWindowFullscreen();
    }
    return fullscreen;
}
