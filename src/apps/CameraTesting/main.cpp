#include "Application.hpp"
#include "CameraController.hpp"
#include "raylib/raylib.h"

class CameraTestingApp : public Application {
private:
    OrbitCamera3D orbitCam;
    FreeFlyCamera3D freeFlyCam;
    TopDownCamera2D topDownCam;

    Vector3 spherePos = { 0.0f, 1.0f, 0.0f };
    Vector2 minimapCircle = { 100.0f, 100.0f };

protected:
    void Init() override {
        // Initialize Cameras
        orbitCam.Init({ 0.0f, 0.0f, 0.0f }, 8.0f);
        freeFlyCam.Init({ 0.0f, 4.0f, 10.0f });
        topDownCam.Init({ 0.0f, 0.0f }, 1.0f);

        // 1. Viewport 1: 3D Orbit Camera (Model Inspector)
        AddSceneViewport(
            "3D Orbit Model Inspector",
            [this]() {
                BeginMode3D(orbitCam.camera);
                    DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                    DrawSphere(spherePos, 0.6f, GOLD);
                    DrawGrid(10, 1.0f);
                EndMode3D();

                DrawText("Orbit Camera: Left Click & Drag to Orbit, Scroll to Zoom", 20, 20, 20, RAYWHITE);
            },
            [this](SceneViewport& svp, float dt) {
                orbitCam.Update(svp, dt);
            }
        );

        // 2. Viewport 2: 3D FreeFly Camera (WASD / QE + Right-Click Look)
        AddSceneViewport(
            "3D FreeFly Camera (WASD)",
            [this]() {
                BeginMode3D(freeFlyCam.camera);
                    DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, GREEN);
                    DrawSphere({ 3.0f, 1.0f, -2.0f }, 0.8f, BLUE);
                    DrawGrid(14, 1.0f);
                EndMode3D();

                DrawText("FreeFly Camera: Left/Right Click & Drag to Look, WASD to Move, Space=Up, LShift=Down", 20, 20, 20, RAYWHITE);
            },
            [this](SceneViewport& svp, float dt) {
                freeFlyCam.Update(svp, dt);
            }
        );

        // 3. Viewport 3: 2D Top-Down Camera (Click & Drag Pan, Scroll Zoom)
        AddSceneViewport(
            "2D Top-Down Camera",
            [this]() {
                BeginMode2D(topDownCam.camera);
                    DrawRectangle(-200, -200, 400, 400, DARKBLUE);
                    DrawCircleV(minimapCircle, 30.0f, YELLOW);
                    DrawGrid(10, 50.0f);
                EndMode2D();

                DrawText("Top-Down 2D Camera: Click & Drag to Pan, Scroll to Zoom", 20, 20, 20, RAYWHITE);
            },
            [this](SceneViewport& svp, float dt) {
                topDownCam.Update(svp, dt);
            }
        );
    }

    // Override onInspectorGui to display camera controls
    void onInspectorGui() override {
        orbitCam.onGui("3D Orbit Camera Controls");
        freeFlyCam.onGui("3D FreeFly Camera Controls");
        topDownCam.onGui("2D Top-Down Camera Controls");
    }

    void Update(float deltaTime) override {
    }

    void Shutdown() override {
    }

public:
    CameraTestingApp() : Application(1280, 720, "Camera Testing Application") {}
};

int main() {
    CameraTestingApp app;
    app.Run();
    return 0;
}
