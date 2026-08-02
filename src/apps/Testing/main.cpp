#include "Application.hpp"
#include "raylib/raylib.h"

class TestApp : public Application {
protected:
    void Init() override {
    }

    void Update(float deltaTime) override {
    }

    void SceneDraw() override {
        DrawText("Scene drawn directly on screen!", 20, 20, 20, RAYWHITE);
        DrawCircle(640, 360, 50.0f, MAROON);
    }

    void Shutdown() override {
    }

public:
    TestApp() : Application(1280, 720, "Minimal Application Test") {}
};

int main() {
    TestApp app;
    app.Run();
    return 0;
}