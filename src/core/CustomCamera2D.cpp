#include "CustomCamera2D.hpp"
#include "raylib/raymath.h"
#include "imgui/imgui.h"

CustomCamera2D::CustomCamera2D(Vector2 target, float zoom) {
    camera.offset = Vector2{ 0.0f, 0.0f };
    camera.target = target;
    camera.rotation = 0.0f;
    camera.zoom = zoom;

    zoomSpeed = 1.0f;
    minZoom = 0.05f;
    maxZoom = 50.0f;
    moveSpeed = 400.0f;
}

void CustomCamera2D::Reset(Vector2 target, float zoom) {
    camera.target = target;
    camera.rotation = 0.0f;
    camera.zoom = zoom;
}

Vector2 CustomCamera2D::GetScreenToWorld(Vector2 screenPos) const {
    return GetScreenToWorld2D(screenPos, camera);
}

Vector2 CustomCamera2D::GetWorldToScreen(Vector2 worldPos) const {
    return GetWorldToScreen2D(worldPos, camera);
}

void CustomCamera2D::Update(float deltaTime, bool isViewportHovered, Vector2 viewportSize) {
    if (viewportSize.x > 0.0f && viewportSize.y > 0.0f) {
        camera.offset = Vector2{ viewportSize.x * 0.5f, viewportSize.y * 0.5f };
    }

    bool allowInput = isViewportHovered && ( IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT));

    if (allowInput) {
        // 1. Mouse Panning (Click & Drag via Left, Right, or Middle mouse button)
        bool isPanningMouse = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || 
                             IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || 
                             IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        if (isPanningMouse) {
            Vector2 delta = GetMouseDelta();
            camera.target.x -= delta.x / camera.zoom;
            camera.target.y -= delta.y / camera.zoom;
        }

        // 2. Keyboard Pan (WASD / Arrow Keys)
        Vector2 moveDelta = { 0.0f, 0.0f };
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    moveDelta.y -= 1.0f;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  moveDelta.y += 1.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  moveDelta.x -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDelta.x += 1.0f;

        if (Vector2LengthSqr(moveDelta) > 0.0f) {
            moveDelta = Vector2Normalize(moveDelta);
            camera.target.x += moveDelta.x * (moveSpeed / camera.zoom) * deltaTime;
            camera.target.y += moveDelta.y * (moveSpeed / camera.zoom) * deltaTime;
        }
    }
    // 3. Mouse Wheel Zooming
    if (isViewportHovered) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            float zoomFactor = 1.0f + wheel * 0.15f * zoomSpeed;
            float newZoom = Clamp(camera.zoom * zoomFactor, minZoom, maxZoom);
            
            if (newZoom != camera.zoom) {
                camera.zoom = newZoom;
            }
        }
    }
}

void CustomCamera2D::Gui() {
    ImGui::Text("2D Camera Controls");
    ImGui::Separator();

    float targetArr[2] = { camera.target.x, camera.target.y };
    if (ImGui::DragFloat2("Target (X, Y)", targetArr, 1.0f, -10000.0f, 10000.0f, "%.1f")) {
        camera.target = Vector2{ targetArr[0], targetArr[1] };
    }

    float offsetArr[2] = { camera.offset.x, camera.offset.y };
    if (ImGui::DragFloat2("Offset (X, Y)", offsetArr, 1.0f, -2000.0f, 2000.0f, "%.1f")) {
        camera.offset = Vector2{ offsetArr[0], offsetArr[1] };
    }

    ImGui::SliderFloat("Zoom", &camera.zoom, minZoom, maxZoom, "%.2fx", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Rotation", &camera.rotation, -180.0f, 180.0f, "%.1f deg");

    ImGui::Separator();
    ImGui::SliderFloat("Pan Speed", &moveSpeed, 50.0f, 2000.0f, "%.0f px/s");
    ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.1f, 3.0f, "%.1f");

    ImGui::Spacing();
    if (ImGui::Button("Reset Camera View", ImVec2(-1, 0))) {
        Reset({ 0.0f, 0.0f }, 1.0f);
    }
}
