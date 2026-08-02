#include "Application.hpp"
#include "raylib/raylib.h"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"

Application::Application(int width, int height, const char* title)
    : width(width), height(height), title(title), running(true),
      m_targetFps(60), m_lastTargetFps(60), m_frameTimeIndex(0) {
    for (int i = 0; i < 100; ++i) {
        m_frameTimeHistory[i] = 0.0f;
    }
}

Application::~Application() {
}

void Application::performanceGui() {
    ImGui::SetNextWindowPos(ImVec2(20.0f, 40.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280.0f, 220.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms/frame ", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::SliderInt("Target FPS", &m_targetFps, 30, 300);
    ImGui::Separator();
    ImGui::PlotHistogram("Frame Times", m_frameTimeHistory, 100, m_frameTimeIndex, nullptr, 0.0f, 33.3f, ImVec2(0, 80));
    ImGui::End();
}

void Application::Run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title);
    SetTargetFPS(60);

    rlImGuiSetup(true);

    Init();

    while (!WindowShouldClose() && running) {
        float deltaTime = GetFrameTime();

        if (m_targetFps != m_lastTargetFps) {
            SetTargetFPS(m_targetFps);
            m_lastTargetFps = m_targetFps;
        }

        m_frameTimeHistory[m_frameTimeIndex] = deltaTime * 1000.0f;
        m_frameTimeIndex = (m_frameTimeIndex + 1) % 100;

        Update(deltaTime);

        BeginDrawing();
            ClearBackground(DARKGRAY);

            // Draw scene directly on screen
            SceneDraw();

            // ImGui UI Overlay
            rlImGuiBegin();

            performanceGui();

            rlImGuiEnd();

        EndDrawing();
    }

    Shutdown();
    rlImGuiShutdown();
    CloseWindow();
}
