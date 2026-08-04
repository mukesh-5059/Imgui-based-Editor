#include "Application.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"

class InspectorTestingApp : public Application {
private:
    Camera3D camera3D = { 0 };

    // Inspectable Object Properties
    Vector3 cubePos = { 0.0f, 1.0f, 0.0f };
    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };
    Color cubeColor = RED;
    ImVec4 cubeColorImGui = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    Vector3 spherePos = { 3.0f, 1.0f, 0.0f };
    float sphereRadius = 1.0f;
    Color sphereColor = GOLD;
    ImVec4 sphereColorImGui = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);

    bool drawWireframe = false;
    bool showDebugPanel = true;

protected:
    void Init() override {
        // Setup 3D Camera
        camera3D.position = (Vector3){ 0.0f, 8.0f, 12.0f };
        camera3D.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
        camera3D.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
        camera3D.fovy     = 45.0f;
        camera3D.projection = CAMERA_PERSPECTIVE;

        // Register 3D Viewport
        AddSceneViewport(
            "3D Inspector Viewport",
            [this]() {
                BeginMode3D(camera3D);
                    if (drawWireframe) {
                        DrawCubeWiresV(cubePos, cubeSize, cubeColor);
                        DrawSphereWires(spherePos, sphereRadius, 16, 16, sphereColor);
                    } else {
                        DrawCubeV(cubePos, cubeSize, cubeColor);
                        DrawSphere(spherePos, sphereRadius, sphereColor);
                    }
                    DrawGrid(12, 1.0f);
                EndMode3D();

                DrawText("Inspector Testing App - Tweak properties in Inspector Window!", 20, 20, 20, RAYWHITE);
            }
        );
    }

    // Override virtual onInspectorGui to display & edit properties inside the Inspector window
    void onInspectorGui() override {
        if (ImGui::CollapsingHeader("Rendering Options", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Wireframe Mode", &drawWireframe);
            ImGui::SliderFloat("Camera FOV", &camera3D.fovy, 20.0f, 120.0f);
        }

        if (ImGui::CollapsingHeader("Red Cube Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position##cube", &cubePos.x, 0.05f);
            ImGui::DragFloat3("Size##cube", &cubeSize.x, 0.05f, 0.1f, 10.0f);
            if (ImGui::ColorEdit4("Cube Color", (float*)&cubeColorImGui)) {
                cubeColor = Color{ (unsigned char)(cubeColorImGui.x * 255),
                                   (unsigned char)(cubeColorImGui.y * 255),
                                   (unsigned char)(cubeColorImGui.z * 255),
                                   (unsigned char)(cubeColorImGui.w * 255) };
            }
        }

        if (ImGui::CollapsingHeader("Gold Sphere Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position##sphere", &spherePos.x, 0.05f);
            ImGui::SliderFloat("Radius##sphere", &sphereRadius, 0.1f, 5.0f);
            if (ImGui::ColorEdit4("Sphere Color", (float*)&sphereColorImGui)) {
                sphereColor = Color{ (unsigned char)(sphereColorImGui.x * 255),
                                     (unsigned char)(sphereColorImGui.y * 255),
                                     (unsigned char)(sphereColorImGui.z * 255),
                                     (unsigned char)(sphereColorImGui.w * 255) };
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Reset All Objects", ImVec2(-1, 0))) {
            cubePos = { 0.0f, 1.0f, 0.0f };
            cubeSize = { 2.0f, 2.0f, 2.0f };
            spherePos = { 3.0f, 1.0f, 0.0f };
            sphereRadius = 1.0f;
        }
    }

    // Override virtual onCustomGui to render custom application windows
    void onCustomGui() override {
        if (showDebugPanel) {
            if (ImGui::Begin("Custom Debug Tools", &showDebugPanel)) {
                ImGui::Text("Rendered via Application::onCustomGui()");
                ImGui::Separator();
                ImGui::Text("Cube World Coords: (%.2f, %.2f, %.2f)", cubePos.x, cubePos.y, cubePos.z);
                ImGui::Text("Sphere World Coords: (%.2f, %.2f, %.2f)", spherePos.x, spherePos.y, spherePos.z);

                if (ImGui::Button("Log Info to Console")) {
                    ConsoleLog::Get().AddLog(LogLevel::Info, "Cube Pos: (%.2f, %.2f, %.2f)", cubePos.x, cubePos.y, cubePos.z);
                }
            }
            ImGui::End();
        }
    }

    void Update(float deltaTime) override {}
    void Shutdown() override {}

public:
    InspectorTestingApp() : Application(1280, 720, "Inspector Testing Application") {}
};

int main() {
    InspectorTestingApp app;
    app.Run();
    return 0;
}
