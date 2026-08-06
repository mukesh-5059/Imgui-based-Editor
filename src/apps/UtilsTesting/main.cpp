#include "Application.hpp"
#include "CameraController.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

class UtilsTestingApp : public Application {
private:
    OrbitCamera3D orbitCam;

    // Interactive 3D Objects for Raycast Testing
    Vector3 cubePos = { -2.0f, 1.0f, 0.0f };
    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };
    Color cubeColor = RED;

    Vector3 spherePos = { 2.0f, 1.0f, 0.0f };
    float sphereRadius = 1.2f;
    Color sphereColor = GOLD;

    // Live Viewport Utility Test States
    Vector2 lastNormPos = { 0.0f, 0.0f };
    Vector2 lastLocalPos = { 0.0f, 0.0f };
    Vector2 lastClickNorm = { 0.0f, 0.0f };
    Vector2 lastClickLocal = { 0.0f, 0.0f };
    bool cubeHit = false;
    bool sphereHit = false;
    int clickCount = 0;
    int doubleClickCount = 0;

protected:
    void Init() override {
        // Initialize 3D Orbit Camera
        orbitCam.Init({ 0.0f, 0.0f, 0.0f }, 10.0f);

        // Register Viewport for Utility Testing
        AddSceneViewport(
            "Viewport Utilities Test",
            [this]() {
                BeginMode3D(orbitCam.camera);
                    // Draw Cube (Turns Green on Raycast Hit)
                    Color activeCubeColor = cubeHit ? GREEN : cubeColor;
                    DrawCubeV(cubePos, cubeSize, activeCubeColor);
                    DrawCubeWiresV(cubePos, cubeSize, WHITE);

                    // Draw Sphere (Turns Green on Raycast Hit)
                    Color activeSphereColor = sphereHit ? GREEN : sphereColor;
                    DrawSphere(spherePos, sphereRadius, activeSphereColor);
                    DrawSphereWires(spherePos, sphereRadius, 16, 16, WHITE);

                    DrawGrid(12, 1.0f);
                EndMode3D();

                // Live HUD Overlay displaying Viewport Utility metrics
                DrawRectangle(10, 10, 480, 150, Fade(BLACK, 0.75f));
                DrawRectangleLines(10, 10, 480, 150, GREEN);

                DrawText("--- VIEWPORT UTILITIES REAL-TIME HUD ---", 20, 20, 16, YELLOW);
                DrawText(TextFormat("Mouse Pos Norm : (%.3f, %.3f)", lastNormPos.x, lastNormPos.y), 20, 45, 16, RAYWHITE);
                DrawText(TextFormat("Mouse Pos Local: (%.1f, %.1f) px", lastLocalPos.x, lastLocalPos.y), 20, 65, 16, RAYWHITE);
                DrawText(TextFormat("Last Click Norm: (%.3f, %.3f)", lastClickNorm.x, lastClickNorm.y), 20, 85, 16, RAYWHITE);
                DrawText(TextFormat("Total Clicks: %d | Double Clicks: %d", clickCount, doubleClickCount), 20, 105, 16, RAYWHITE);
                DrawText(TextFormat("3D Raycast Hit : %s", (cubeHit ? "RED CUBE" : (sphereHit ? "GOLD SPHERE" : "NONE"))), 20, 125, 16, (cubeHit || sphereHit) ? GREEN : RED);
            },
            [this](SceneViewport& svp, float dt) {
                // 1. Test Orbit Camera update
                orbitCam.Update(svp, dt);

                // 2. Test GetViewportMousePosNorm & GetViewportMousePosLocal
                lastNormPos = GetViewportMousePosNorm(svp);
                lastLocalPos = GetViewportMousePosLocal(svp);

                // 3. Test GetViewportMouseRay (3D Raycasting)
                Ray ray = GetViewportMouseRay(svp, orbitCam.camera);
                
                BoundingBox cubeBox = {
                    Vector3Subtract(cubePos, Vector3Scale(cubeSize, 0.5f)),
                    Vector3Add(cubePos, Vector3Scale(cubeSize, 0.5f))
                };
                RayCollision cHit = GetRayCollisionBox(ray, cubeBox);
                RayCollision sHit = GetRayCollisionSphere(ray, spherePos, sphereRadius);

                cubeHit = cHit.hit;
                sphereHit = sHit.hit;

                // 4. Test IsViewportClicked & GetViewportClickPosNorm/Local
                if (IsViewportClicked(svp, MOUSE_BUTTON_LEFT)) {
                    clickCount++;
                    lastClickNorm = GetViewportClickPosNorm(svp);
                    lastClickLocal = GetViewportClickPosLocal(svp);

                    ConsoleLog::Get().AddLog(
                        LogLevel::Info,
                        "[Click #%d] Viewport Clicked at Norm: (%.3f, %.3f) Local: (%.1f, %.1f) px",
                        clickCount, lastClickNorm.x, lastClickNorm.y, lastClickLocal.x, lastClickLocal.y
                    );
                }

                // 5. Test IsViewportDoubleClicked
                if (IsViewportDoubleClicked(svp, MOUSE_BUTTON_LEFT)) {
                    doubleClickCount++;
                    ConsoleLog::Get().AddLog(LogLevel::Warning, "[Double Click #%d] Double clicked viewport!", doubleClickCount);

                    // Toggle colors on double click
                    cubeColor = (cubeColor.r == 255) ? PURPLE : RED;
                }
            }
        );
    }

    void onInspectorGui() override {
        orbitCam.onGui("Orbit Camera Controls");

        if (ImGui::CollapsingHeader("Viewport Utility Test Metrics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Mouse Pos Norm: (%.3f, %.3f)", lastNormPos.x, lastNormPos.y);
            ImGui::Text("Mouse Pos Local: (%.1f, %.1f) px", lastLocalPos.x, lastLocalPos.y);
            ImGui::Text("Last Click Norm: (%.3f, %.3f)", lastClickNorm.x, lastClickNorm.y);
            ImGui::Text("Last Click Local: (%.1f, %.1f) px", lastClickLocal.x, lastClickLocal.y);
            ImGui::Separator();
            ImGui::Text("Click Count: %d", clickCount);
            ImGui::Text("Double Click Count: %d", doubleClickCount);
            ImGui::Text("Raycast Target: %s", (cubeHit ? "Red Cube" : (sphereHit ? "Gold Sphere" : "None")));

            if (ImGui::Button("Reset Counters", ImVec2(-1, 0))) {
                clickCount = 0;
                doubleClickCount = 0;
            }
        }
    }

    void Update(float deltaTime) override {}
    void Shutdown() override {}

public:
    UtilsTestingApp() : Application(1280, 720, "Viewport Utilities Testing Application") {}
};

int main() {
    UtilsTestingApp app;
    app.Run();
    return 0;
}
