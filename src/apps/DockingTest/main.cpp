#include "raylib/raylib.h"
#include "rlimgui/rlImGui.h"
#include "imgui/imgui.h"

int main() {
    // 1. Initialize Raylib window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Dear ImGui Native Docking Test App");
    SetTargetFPS(60);

    // 2. Initialize rlImGui backend
    rlImGuiSetup(true);

    // 3. Enable ImGui Docking Flag
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Render loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        rlImGuiBegin();

        // 4. Submit DockSpace over the entire main viewport
        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspaceFlags);

        // 5. Submit Dockable Window 1: Viewport
        ImGui::Begin("Viewport");
        ImGui::Text("Main Viewport Container");
        ImGui::Text("Current FPS: %d", GetFPS());
        ImGui::Separator();
        ImGui::Text("Drag window tab headers to dock/undock panels!");
        ImGui::End();

        // 6. Submit Dockable Window 2: Inspector
        ImGui::Begin("Inspector");
        ImGui::Text("Properties & Controls");
        static float color[4] = { 0.2f, 0.7f, 0.9f, 1.0f };
        ImGui::ColorEdit4("Accent Color", color);
        static float scale = 1.0f;
        ImGui::SliderFloat("Scale", &scale, 0.1f, 5.0f);
        ImGui::End();

        // 7. Submit Dockable Window 3: Console Log
        ImGui::Begin("Console Log");
        ImGui::Text("[INFO] ImGui Native Docking Test App initialized.");
        ImGui::Text("[INFO] DockSpace ID: %u", dockspaceId);
        ImGui::Text("[INFO] Docking branch enabled: %s", (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) ? "YES" : "NO");
        ImGui::End();

        // 8. Submit Dockable Window 4: ImGui Demo Window
        static bool showDemo = true;
        if (showDemo) {
            ImGui::ShowDemoWindow(&showDemo);
        }

        rlImGuiEnd();

        EndDrawing();
    }

    // Cleanup
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
