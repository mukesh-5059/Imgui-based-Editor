#include "Application.hpp"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"

Application::Application(int width, int height, const char* title)
    : width(width), height(height), title(title), running(true),
      m_targetFps(60), m_lastTargetFps(60), m_frameTimeIndex(0),
      showPerformance(false),
      nextSceneViewportId(1), activeSceneViewportIndex(0) {
    for (int i = 0; i < 100; ++i) {
        m_frameTimeHistory[i] = 0.0f;
    }
}

Application::~Application() {
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
            ImGui::MenuItem("Performance Profiler", nullptr, &showPerformance);
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

void Application::Run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title);
    SetTargetFPS(60);

    rlImGuiSetup(true);

    // Enable Dear ImGui Native Docking (Layouts loaded & saved via imgui.ini automatically)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

        sceneViewportsRender();

        // 2. Main Screen Rendering & ImGui DockSpace Pass
        BeginDrawing();
            ClearBackground(BLACK);

            rlImGuiBegin();

            renderMainMenuBar();

            // Submit Central DockSpace over Main Viewport (ImGui loads/saves layout to imgui.ini automatically)
            ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspaceFlags);

            sceneViewportInputs(deltaTime);

            performanceGui();

            rlImGuiEnd();

        EndDrawing();
    }

    Shutdown();
    rlImGuiShutdown();
    CloseWindow();
}
