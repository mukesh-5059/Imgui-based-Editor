#include "Application.hpp"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "utils.hpp"

int Application::AddSceneViewport(const std::string& name,
                                  std::function<void()> drawCallback,
                                  std::function<void(SceneViewport& svp, float dt)> inputCallback,
                                  std::function<void(SceneViewport&, Vector2 normPos)> onClick,
                                  bool canClose,
                                  void* userData) {
    SceneViewport svp;
    svp.id = nextSceneViewportId++;
    svp.name = name.empty() ? ("Viewport " + std::to_string(svp.id)) : name;
    svp.canClose = canClose;
    svp.userData = userData;
    svp.drawCallback = drawCallback;
    svp.inputCallback = inputCallback;
    svp.clickCallback = onClick;

    if (width > 0 && height > 0) {
        svp.renderTexture = LoadRenderTexture(width, height);
        SetTextureFilter(svp.renderTexture.texture, TEXTURE_FILTER_BILINEAR);
        svp.isLoaded = true;
    }

    sceneViewports.push_back(svp);
    activeSceneViewportIndex = (int)sceneViewports.size() - 1;
    return svp.id;
}

void Application::RemoveSceneViewport(int index) {
    if (index >= 0 && index < (int)sceneViewports.size()) {
        if (sceneViewports[index].isLoaded && sceneViewports[index].renderTexture.id > 0) {
            UnloadRenderTexture(sceneViewports[index].renderTexture);
        }
        sceneViewports[index].isLoaded = false;
        sceneViewports.erase(sceneViewports.begin() + index);

        if (activeSceneViewportIndex >= (int)sceneViewports.size()) {
            activeSceneViewportIndex = (int)sceneViewports.size() - 1;
        }
        if (activeSceneViewportIndex < 0) activeSceneViewportIndex = 0;
    }
}

void Application::sceneViewportsRender() {
    // 1. Offscreen Scene Pass (Renders offscreen IF visible on screen, hovered, active, or on initial draw)
    for (size_t i = 0; i < sceneViewports.size(); ++i) {
        auto& svp = sceneViewports[i];
        if (!svp.isLoaded || svp.renderTexture.id == 0) {
            svp.renderTexture = LoadRenderTexture(width, height);
            SetTextureFilter(svp.renderTexture.texture, TEXTURE_FILTER_BILINEAR);
            svp.isLoaded = true;
        }
        if (svp.isVisible || svp.isHovered || svp.isActive || !svp.hasDrawnOnce) {
            BeginTextureMode(svp.renderTexture);
                ClearBackground(DARKGRAY);
                if (svp.drawCallback) {
                    svp.drawCallback();
                }
            EndTextureMode();
            svp.hasDrawnOnce = true;
        }
    }
}

void Application::sceneViewportInputs(float dt) {
    int removeIndex = -1;
    for (size_t i = 0; i < sceneViewports.size(); ++i) {
        auto& svp = sceneViewports[i];
        bool* openPtr = svp.canClose ? &svp.open : nullptr;

        if (ImGui::Begin(svp.name.c_str(), openPtr)) {
            svp.isVisible = true;

            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 0 && avail.y > 0 && svp.isLoaded && svp.renderTexture.id > 0) {
                rlImGuiImageRenderTextureFit(&svp.renderTexture, true);

                bool isHovered = ImGui::IsItemHovered();
                bool isActive  = ImGui::IsItemActive();

                svp.isHovered = isHovered;
                svp.isActive  = isActive;
                svp.isClicked = ImGui::IsItemClicked();
                svp.isDoubleClicked = isHovered && ImGui::IsMouseDoubleClicked(0);

                if (isHovered || isActive) {
                    ImVec2 minP = ImGui::GetItemRectMin();
                    ImVec2 sizeP = ImGui::GetItemRectSize();
                    ImVec2 mouseP = ImGui::GetMousePos();

                    if (sizeP.x > 0.0f && sizeP.y > 0.0f) {
                        svp.mousePosNorm.x = (mouseP.x - minP.x) / sizeP.x;
                        svp.mousePosNorm.y = (mouseP.y - minP.y) / sizeP.y;
                        svp.mousePosLocal.x = mouseP.x - minP.x;
                        svp.mousePosLocal.y = mouseP.y - minP.y;
                    }

                    if (svp.isClicked) {
                        svp.clickPosNorm = svp.mousePosNorm;
                        svp.clickPosLocal = svp.mousePosLocal;
                        if (svp.clickCallback) {
                            svp.clickCallback(svp, svp.clickPosNorm);
                        }
                    }

                    if (svp.inputCallback) {
                        svp.inputCallback(svp, dt);
                    }
                }
            }
        } else {
            svp.isVisible = false;
            svp.isHovered = false;
            svp.isActive  = false;
            svp.isClicked = false;
            svp.isDoubleClicked = false;
        }
        ImGui::End();

        if (svp.canClose && !svp.open) {
            removeIndex = (int)i;
        }
    }

    if (removeIndex >= 0) {
        RemoveSceneViewport(removeIndex);
    }
}

SceneViewport* Application::GetActiveSceneViewport() {
    if (activeSceneViewportIndex >= 0 && activeSceneViewportIndex < (int)sceneViewports.size()) {
        return &sceneViewports[activeSceneViewportIndex];
    }
    return nullptr;
}

SceneViewport* Application::GetSceneViewport(int viewportId) {
    for (auto& svp : sceneViewports) {
        if (svp.id == viewportId) return &svp;
    }
    return nullptr;
}

// Standalone Viewport Helper Functions
Vector2 GetViewportMousePosNorm(const SceneViewport& svp) {
    return svp.mousePosNorm;
}

Vector2 GetViewportMousePosLocal(const SceneViewport& svp) {
    return svp.mousePosLocal;
}

Vector2 GetViewportClickPosNorm(const SceneViewport& svp) {
    return svp.clickPosNorm;
}

Vector2 GetViewportClickPosLocal(const SceneViewport& svp) {
    return svp.clickPosLocal;
}

Ray GetViewportMouseRay(const SceneViewport& svp, const Camera3D& camera) {
    int vpWidth = svp.renderTexture.texture.width > 0 ? svp.renderTexture.texture.width : 1280;
    int vpHeight = svp.renderTexture.texture.height > 0 ? svp.renderTexture.texture.height : 720;

    Vector2 screenPos = {
        svp.mousePosNorm.x * (float)vpWidth,
        svp.mousePosNorm.y * (float)vpHeight
    };

    return GetScreenToWorldRayEx(screenPos, camera, vpWidth, vpHeight);
}

bool IsViewportClicked(const SceneViewport& svp, MouseButton button) {
    return svp.isClicked && IsMouseButtonPressed(button);
}

bool IsViewportDoubleClicked(const SceneViewport& svp, MouseButton button) {
    return svp.isDoubleClicked;
}
