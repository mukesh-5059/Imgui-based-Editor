#pragma once
#include "raylib/raylib.h"

class CustomCamera2D {
public:
    Camera2D camera;

    float zoomSpeed;   // Mouse wheel zoom sensitivity factor
    float minZoom;     // Minimum zoom level limit
    float maxZoom;     // Maximum zoom level limit
    float moveSpeed;   // Keyboard movement speed (units per sec)

    CustomCamera2D(Vector2 target = {0.0f, 0.0f}, float zoom = 1.0f);

    void Update(float deltaTime, bool isViewportHovered = true, Vector2 viewportSize = {0.0f, 0.0f});
    void Gui();

    Vector2 GetScreenToWorld(Vector2 screenPos) const;
    Vector2 GetWorldToScreen(Vector2 worldPos) const;
    void Reset(Vector2 target = {0.0f, 0.0f}, float zoom = 1.0f);
};
