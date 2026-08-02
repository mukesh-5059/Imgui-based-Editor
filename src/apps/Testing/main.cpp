#include "Application.hpp"
#include "raylib/raylib.h"

class ViewportTestingApp : public Application {
private:
    Camera3D camera3D = { 0 };
    float cubeRotation = 0.0f;
    Vector2 circlePos = { 400.0f, 300.0f };

    // Movable sphere state
    Vector3 spherePos = { -2.0f, 0.5f, 0.0f };
    float sphereSpeed = 3.0f;

protected:
    void Init() override {
        // Setup 3D Perspective Camera
        camera3D.position = (Vector3){ 0.0f, 6.0f, 10.0f };
        camera3D.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
        camera3D.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
        camera3D.fovy     = 45.0f;
        camera3D.projection = CAMERA_PERSPECTIVE;

        // Register Viewport 1 with 3D draw & input callbacks
        AddSceneViewport(
            "3D Viewport",
            [this]() {
                BeginMode3D(camera3D);
                    DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                    DrawSphere(spherePos, 0.6f, GOLD);
                    DrawGrid(10, 1.0f);
                EndMode3D();

                DrawText("3D Viewport (Bouncing Gold Sphere)", 20, 20, 20, RAYWHITE);
            },
            [this](SceneViewport& svp, float dt) {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    cubeRotation += GetMouseDelta().x * 0.5f;
                }
            }
        );

        // Register Viewport 2 with 2D draw & input callbacks
        AddSceneViewport(
            "2D Minimap View",
            [this]() {
                DrawText("2D Minimap (Click & Drag Blue Circle)", 20, 20, 20, YELLOW);
                DrawCircleV(circlePos, 40.0f, BLUE);
            },
            [this](SceneViewport& svp, float dt) {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    circlePos.x += GetMouseDelta().x;
                    circlePos.y += GetMouseDelta().y;
                }
            }
        );
    }

    void Update(float deltaTime) override {
        // Continuously animate movable sphere position
        spherePos.x += sphereSpeed * deltaTime;
        if (spherePos.x > 3.0f) {
            spherePos.x = 3.0f;
            sphereSpeed *= -1.0f;
        } else if (spherePos.x < -3.0f) {
            spherePos.x = -3.0f;
            sphereSpeed *= -1.0f;
        }
    }

    void Shutdown() override {
    }

public:
    ViewportTestingApp() : Application(1280, 720, "Viewport Testing Application") {}
};

int main() {
    ViewportTestingApp app;
    app.Run();
    return 0;
}