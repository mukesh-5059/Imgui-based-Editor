#pragma once
#include "raylib/raylib.h"
#include "imgui/imgui.h"
#include "utils.hpp"

// 1. Orbit 3D Camera (Model Inspector Camera - Orbits around target/origin)
struct OrbitCamera3D {
    Camera3D camera = { 0 };
    Vector3 target = { 0.0f, 0.0f, 0.0f };
    float distance = 10.0f;
    float alpha = 0.0f;       // Horizontal azimuth angle (radians)
    float beta = 0.4f;        // Vertical elevation angle (radians)
    float rotationSpeed = 0.005f;
    float zoomSpeed = 1.0f;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;

    void Init(Vector3 targetPos = { 0.0f, 0.0f, 0.0f }, float initialDistance = 10.0f);
    void Update(SceneViewport& svp, float dt);
    void ApplyMatrix();
    void onGui(const char* label = "Orbit Camera 3D");
};

// 2. FreeFly 3D Camera (First-Person / WASD Camera - Rotates via left/right click drag)
struct FreeFlyCamera3D {
    Camera3D camera = { 0 };
    float yaw = -90.0f;
    float pitch = -20.0f;
    float moveSpeed = 5.0f;
    float sensitivity = 0.15f;

    void Init(Vector3 startPos = { 0.0f, 5.0f, 10.0f });
    void Update(SceneViewport& svp, float dt);
    void onGui(const char* label = "FreeFly Camera 3D");
};

// 3. Top-Down 2D Camera (Click & Drag Panning, Mouse Wheel Zoom)
struct TopDownCamera2D {
    Camera2D camera = { 0 };
    float zoomSpeed = 0.1f;
    float minZoom = 0.1f;
    float maxZoom = 10.0f;

    void Init(Vector2 targetPos = { 0.0f, 0.0f }, float initialZoom = 1.0f);
    void Update(SceneViewport& svp, float dt);
    void onGui(const char* label = "TopDown Camera 2D");
};
