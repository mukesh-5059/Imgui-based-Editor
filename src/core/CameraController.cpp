#include "CameraController.hpp"
#include <cmath>

// --- 1. Orbit3D Camera Implementation ---
void OrbitCamera3D::Init(Vector3 targetPos, float initialDistance) {
    target = targetPos;
    distance = initialDistance;
    alpha = 0.0f;
    beta = 0.4f;

    camera.target = target;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    ApplyMatrix();
}

void OrbitCamera3D::ApplyMatrix() {
    if (beta > 1.55f) beta = 1.55f;
    if (beta < -1.55f) beta = -1.55f;

    camera.target = target;
    camera.position.x = target.x + distance * cosf(beta) * sinf(alpha);
    camera.position.y = target.y + distance * sinf(beta);
    camera.position.z = target.z + distance * cosf(beta) * cosf(alpha);
}

void OrbitCamera3D::Update(SceneViewport& svp, float dt) {
    if (!svp.isHovered && !svp.isActive) return;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        alpha -= delta.x * rotationSpeed;
        beta += delta.y * rotationSpeed;
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        distance -= wheel * zoomSpeed;
        if (distance < minDistance) distance = minDistance;
        if (distance > maxDistance) distance = maxDistance;
    }

    ApplyMatrix();
}

void OrbitCamera3D::onGui(const char* label) {
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::DragFloat3("Target Pos", &target.x, 0.05f)) {
            ApplyMatrix();
        }
        if (ImGui::SliderFloat("Distance", &distance, minDistance, maxDistance)) {
            ApplyMatrix();
        }
        if (ImGui::SliderFloat("Azimuth (Alpha)", &alpha, -3.14159f, 3.14159f)) {
            ApplyMatrix();
        }
        if (ImGui::SliderFloat("Elevation (Beta)", &beta, -1.55f, 1.55f)) {
            ApplyMatrix();
        }
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.001f, 0.02f);
        ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.1f, 5.0f);
        ImGui::SliderFloat("Field of View", &camera.fovy, 10.0f, 120.0f);
    }
}

// --- 2. FreeFly3D Camera Implementation ---
void FreeFlyCamera3D::Init(Vector3 startPos) {
    camera.position = startPos;
    camera.target = (Vector3){ startPos.x, startPos.y, startPos.z - 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    yaw = -90.0f;
    pitch = -10.0f;
}

void FreeFlyCamera3D::Update(SceneViewport& svp, float dt) {
    if (!svp.isHovered && !svp.isActive) return;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        yaw += delta.x * sensitivity;
        pitch -= delta.y * sensitivity;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    float radYaw = DEG2RAD * yaw;
    float radPitch = DEG2RAD * pitch;

    Vector3 forward = {
        cosf(radPitch) * cosf(radYaw),
        sinf(radPitch),
        cosf(radPitch) * sinf(radYaw)
    };

    Vector3 right = {
        -sinf(radYaw),
        0.0f,
        cosf(radYaw)
    };

    float currentSpeed = moveSpeed * dt;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_SHIFT)) currentSpeed *= 2.0f;

    if (IsKeyDown(KEY_W)) {
        camera.position.x += forward.x * currentSpeed;
        camera.position.y += forward.y * currentSpeed;
        camera.position.z += forward.z * currentSpeed;
    }
    if (IsKeyDown(KEY_S)) {
        camera.position.x -= forward.x * currentSpeed;
        camera.position.y -= forward.y * currentSpeed;
        camera.position.z -= forward.z * currentSpeed;
    }
    if (IsKeyDown(KEY_D)) {
        camera.position.x += right.x * currentSpeed;
        camera.position.z += right.z * currentSpeed;
    }
    if (IsKeyDown(KEY_A)) {
        camera.position.x -= right.x * currentSpeed;
        camera.position.z -= right.z * currentSpeed;
    }
    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_E)) camera.position.y += currentSpeed;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_Q)) camera.position.y -= currentSpeed;

    camera.target = {
        camera.position.x + forward.x,
        camera.position.y + forward.y,
        camera.position.z + forward.z
    };
}

void FreeFlyCamera3D::onGui(const char* label) {
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Position", &camera.position.x, 0.05f);
        ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f);
        ImGui::SliderFloat("Move Speed", &moveSpeed, 0.5f, 50.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &sensitivity, 0.01f, 1.0f);
        ImGui::SliderFloat("Field of View", &camera.fovy, 10.0f, 120.0f);
    }
}

// --- 3. TopDown2D Camera Implementation ---
void TopDownCamera2D::Init(Vector2 targetPos, float initialZoom) {
    camera.target = targetPos;
    camera.offset = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = initialZoom;
}

void TopDownCamera2D::Update(SceneViewport& svp, float dt) {
    if (!svp.isHovered && !svp.isActive) return;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        camera.target.x -= delta.x / camera.zoom;
        camera.target.y -= delta.y / camera.zoom;
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        camera.zoom += wheel * zoomSpeed * camera.zoom;
        if (camera.zoom < minZoom) camera.zoom = minZoom;
        if (camera.zoom > maxZoom) camera.zoom = maxZoom;
    }
}

void TopDownCamera2D::onGui(const char* label) {
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Target Pos", &camera.target.x, 0.5f);
        ImGui::DragFloat2("Offset", &camera.offset.x, 0.5f);
        ImGui::SliderFloat("Zoom Level", &camera.zoom, minZoom, maxZoom);
        ImGui::SliderFloat("Rotation Angle", &camera.rotation, -180.0f, 180.0f);
        ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.01f, 0.5f);
    }
}
