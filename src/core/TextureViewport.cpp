#include "Application.hpp"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "ConsoleLog.hpp"
#include "FilePicker.hpp"
#include "utils.hpp"
#include <cstdio>

int Application::AddTextureViewport(const std::string& initialPath) {
    TextureViewport vp;
    vp.id = nextViewportId++;
    vp.name = "Texture " + std::to_string(vp.id);
    vp.ownsTexture = true;
    if (!initialPath.empty()) {
        snprintf(vp.filePath, sizeof(vp.filePath), "%s", initialPath.c_str());
        if (FileExists(vp.filePath)) {
            vp.texture = LoadTexture(vp.filePath);
            if (vp.texture.id > 0 && vp.texture.width > 0) {
                SetTextureFilter(vp.texture, TEXTURE_FILTER_BILINEAR);
                vp.isLoaded = true;
                vp.loadedPath = vp.filePath;
            }
        }
    }
    textureViewports.push_back(vp);
    return vp.id;
}

int Application::AddTextureViewport(Texture2D texture, const std::string& name, std::function<Texture2D()> reloadCallback, bool ownsTexture, bool canClose) {
    TextureViewport vp;
    vp.id = nextViewportId++;
    vp.name = name.empty() ? ("Texture " + std::to_string(vp.id)) : name;
    vp.texture = texture;
    vp.isLoaded = (texture.id > 0);
    vp.ownsTexture = ownsTexture;
    vp.canClose = canClose;
    vp.reloadCallback = reloadCallback;
    if (vp.isLoaded) {
        SetTextureFilter(vp.texture, TEXTURE_FILTER_BILINEAR);
        vp.loadedPath = "[Raylib Texture2D]";
    }
    textureViewports.push_back(vp);
    return vp.id;
}

void Application::RemoveTextureViewport(int index) {
    if (index >= 0 && index < (int)textureViewports.size()) {
        if (textureViewports[index].isLoaded && textureViewports[index].ownsTexture && textureViewports[index].texture.id > 0) {
            UnloadTexture(textureViewports[index].texture);
        }
        textureViewports[index].isLoaded = false;
        textureViewports.erase(textureViewports.begin() + index);
    }
}

void Application::SetTextureViewportCallback(int viewportId, std::function<Texture2D()> reloadCallback) {
    for (auto& vp : textureViewports) {
        if (vp.id == viewportId) {
            vp.reloadCallback = reloadCallback;
            break;
        }
    }
}

void Application::SetTextureViewportTexture(int viewportId, Texture2D texture, bool ownsTexture) {
    for (auto& vp : textureViewports) {
        if (vp.id == viewportId) {
            if (vp.isLoaded && vp.ownsTexture && vp.texture.id > 0) {
                UnloadTexture(vp.texture);
            }
            vp.texture = texture;
            vp.isLoaded = (texture.id > 0);
            vp.ownsTexture = ownsTexture;
            if (vp.isLoaded) {
                SetTextureFilter(vp.texture, TEXTURE_FILTER_BILINEAR);
            }
            break;
        }
    }
}

void Application::ReloadTextureViewport(TextureViewport& vp) {
    Texture2D newTex = { 0 };

    if (vp.reloadCallback) {
        newTex = vp.reloadCallback();
    } else if (vp.filePath[0] != '\0' && FileExists(vp.filePath)) {
        newTex = LoadTexture(vp.filePath);
    }

    if (newTex.id > 0 && newTex.width > 0) {
        if (vp.isLoaded && vp.ownsTexture && vp.texture.id > 0) {
            UnloadTexture(vp.texture);
        }
        vp.texture = newTex;
        vp.isLoaded = true;
        SetTextureFilter(vp.texture, TEXTURE_FILTER_BILINEAR);
        if (vp.reloadCallback) {
            vp.loadedPath = "[Raylib Texture2D Callback]";
        } else {
            vp.loadedPath = vp.filePath;
        }
        ConsoleLog::Get().AddLog(LogLevel::Info, "Texture viewport '%s' reloaded successfully.", vp.name.c_str());
    }
}

void Application::ReloadTextureViewport(int viewportId) {
    for (auto& vp : textureViewports) {
        if (vp.id == viewportId) {
            ReloadTextureViewport(vp);
            break;
        }
    }
}

TextureViewport* Application::GetTextureViewport(int viewportId) {
    for (auto& vp : textureViewports) {
        if (vp.id == viewportId) {
            return &vp;
        }
    }
    return nullptr;
}

void Application::textureViewportInputs(float dt) {
    int removeIndex = -1;

    for (size_t i = 0; i < textureViewports.size(); ++i) {
        auto& vp = textureViewports[i];
        bool* openPtr = vp.canClose ? &vp.open : nullptr;

        if (vp.pendingReload) {
            vp.pendingReload = false;
            ReloadTextureViewport(vp);
        }

        if (ImGui::Begin(vp.name.c_str(), openPtr)) {
            vp.isVisible = true;
            vp.isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
            vp.isActive  = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

            // Bind Shortcut: Ctrl + R when texture viewport window is visible & focused/hovered
            bool isCtrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            if ((vp.isHovered || vp.isActive) && isCtrlDown && IsKeyPressed(KEY_R)) {
                ReloadTextureViewport(vp);
            }

            ImGui::Spacing();
            ImGui::AlignTextToFramePadding();

            if (vp.reloadCallback != nullptr) {
                ImGui::Text("Source: Raylib Texture2D (Callback Registered)");
            } else {
                ImGui::Text("File Path:");
                ImGui::SameLine();
                float buttonGroupWidth = 310.0f;
                float availableInputWidth = ImGui::GetContentRegionAvail().x - buttonGroupWidth;
                if (availableInputWidth < 80.0f) availableInputWidth = 80.0f;
                ImGui::SetNextItemWidth(availableInputWidth);
                ImGui::InputText(("##path_" + std::to_string(vp.id)).c_str(), vp.filePath, sizeof(vp.filePath));

                ImGui::SameLine();
                if (ImGui::Button(("Browse...##browse_" + std::to_string(vp.id)).c_str(), ImVec2(80.0f, 0.0f))) {
                    int vpId = vp.id;
                    FilePicker::Get().Open(
                        "Select Texture Image",
                        "",
                        { ".png", ".jpg", ".jpeg", ".tga", ".bmp" },
                        [this, vpId](const std::string& selectedPath) {
                            TextureViewport* targetVp = GetTextureViewport(vpId);
                            if (targetVp) {
                                snprintf(targetVp->filePath, sizeof(targetVp->filePath), "%s", selectedPath.c_str());
                                targetVp->pendingReload = true;
                            }
                        }
                    );
                }

                ImGui::SameLine();
                if (ImGui::Button(("Load##load_" + std::to_string(vp.id)).c_str(), ImVec2(55.0f, 0.0f))) {
                    if (vp.filePath[0] != '\0') {
                        if (vp.isLoaded && vp.ownsTexture && vp.texture.id > 0) {
                            UnloadTexture(vp.texture);
                            vp.isLoaded = false;
                        }
                        if (FileExists(vp.filePath)) {
                            vp.texture = LoadTexture(vp.filePath);
                            if (vp.texture.id > 0 && vp.texture.width > 0) {
                                SetTextureFilter(vp.texture, TEXTURE_FILTER_BILINEAR);
                                vp.isLoaded = true;
                                vp.ownsTexture = true;
                                vp.loadedPath = vp.filePath;
                                ConsoleLog::Get().AddLog(LogLevel::Info, "Loaded texture file '%s'.", vp.filePath);
                            }
                        }
                    }
                }
            }

            if (vp.reloadCallback != nullptr || vp.filePath[0] != '\0') {
                ImGui::SameLine();
                if (ImGui::Button(("Reload (Ctrl+R)##reload_" + std::to_string(vp.id)).c_str(), ImVec2(120.0f, 0.0f))) {
                    ReloadTextureViewport(vp);
                }
            }

            ImGui::SameLine();
            if (ImGui::Button(("Clear##clear_" + std::to_string(vp.id)).c_str(), ImVec2(55.0f, 0.0f))) {
                if (vp.isLoaded && vp.ownsTexture && vp.texture.id > 0) {
                    UnloadTexture(vp.texture);
                }
                vp.isLoaded = false;
                vp.texture = { 0 };
                vp.filePath[0] = '\0';
                vp.loadedPath.clear();
            }

            ImGui::Separator();

            if (vp.isLoaded && vp.texture.id > 0) {
                ImGui::Text("Loaded Texture: %s (%d x %d px)", vp.loadedPath.c_str(), vp.texture.width, vp.texture.height);
                ImVec2 avail = ImGui::GetContentRegionAvail();
                if (avail.x > 0 && avail.y > 0 && vp.texture.width > 0 && vp.texture.height > 0) {
                    float aspect = (float)vp.texture.width / (float)vp.texture.height;
                    float availAspect = avail.x / avail.y;
                    float w = avail.x;
                    float h = avail.y;
                    if (availAspect > aspect) {
                        w = avail.y * aspect;
                    } else {
                        h = avail.x / aspect;
                    }
                    float offsetX = (avail.x - w) * 0.5f;
                    float offsetY = (avail.y - h) * 0.5f;
                    if (offsetX > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
                    if (offsetY > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
                    rlImGuiImageSize(&vp.texture, (int)w, (int)h);
                }
            } else {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "No texture currently loaded.");
                ImGui::Text("Click 'Browse...' or enter an image file path / dynamic callback, then press Ctrl+R to reload.");
            }
        } else {
            vp.isVisible = false;
            vp.isHovered = false;
            vp.isActive  = false;
        }
        ImGui::End();

        if (vp.canClose && !vp.open) {
            removeIndex = (int)i;
        }
    }

    if (removeIndex >= 0) {
        RemoveTextureViewport(removeIndex);
    }
}
