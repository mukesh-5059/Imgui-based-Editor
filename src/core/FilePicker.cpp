#include "FilePicker.hpp"
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

FilePicker::FilePicker()
    : isOpen(false), dialogTitle("Select File") {
    currentPath = fs::current_path();
}

FilePicker& FilePicker::Get() {
    static FilePicker instance;
    return instance;
}

void FilePicker::Open(const std::string& title,
                      const std::string& initialDir,
                      const std::vector<std::string>& extensions,
                      std::function<void(const std::string&)> onSelect) {
    dialogTitle = title;
    allowedExtensions = extensions;
    onSelectCallback = onSelect;
    selectedFile.clear();

    if (!initialDir.empty() && fs::exists(initialDir) && fs::is_directory(initialDir)) {
        currentPath = fs::path(initialDir);
    } else {
        currentPath = fs::current_path();
    }

    isOpen = true;
}

void FilePicker::Draw() {
    if (!isOpen) return;

    ImGui::OpenPopup(dialogTitle.c_str());
    ImGui::SetNextWindowSize(ImVec2(650, 420), ImGuiCond_FirstUseEver);

    if (ImGui::BeginPopupModal(dialogTitle.c_str(), &isOpen, ImGuiWindowFlags_NoDocking)) {
        
        // Navigation Bar Header
        ImGui::Text("Path: %s", currentPath.string().c_str());

        ImGui::Separator();

        // File & Directory List Region
        ImGui::BeginChild("FilePickerScrollRegion", ImVec2(0, -40), true);

        try {
            // Render parent directory '../' option at top of list
            if (currentPath.has_parent_path() && currentPath != currentPath.root_path()) {
                if (ImGui::Selectable("[DIR] ../", false)) {
                    currentPath = currentPath.parent_path();
                    selectedFile.clear();
                }
            }

            std::vector<fs::directory_entry> directories;
            std::vector<fs::directory_entry> files;

            for (const auto& entry : fs::directory_iterator(currentPath, fs::directory_options::skip_permission_denied)) {
                if (entry.is_directory()) {
                    directories.push_back(entry);
                } else if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    if (allowedExtensions.empty()) {
                        files.push_back(entry);
                    } else {
                        for (const auto& allowed : allowedExtensions) {
                            std::string allowedLower = allowed;
                            std::transform(allowedLower.begin(), allowedLower.end(), allowedLower.begin(), ::tolower);
                            if (ext == allowedLower) {
                                files.push_back(entry);
                                break;
                            }
                        }
                    }
                }
            }

            std::sort(directories.begin(), directories.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.path().filename().string() < b.path().filename().string();
            });
            std::sort(files.begin(), files.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.path().filename().string() < b.path().filename().string();
            });

            // Render Sub-Directories (Clicking enters directory)
            for (const auto& dir : directories) {
                std::string dirName = "[DIR] " + dir.path().filename().string() + "/";
                if (ImGui::Selectable(dirName.c_str(), false)) {
                    currentPath = dir.path();
                    selectedFile.clear();
                    break;
                }
            }

            // Render Regular Files (Clicking selects, double-clicking confirms)
            for (const auto& file : files) {
                std::string fileName = file.path().filename().string();
                bool isSelected = (selectedFile == file.path().string());

                if (ImGui::Selectable(fileName.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                    selectedFile = file.path().string();

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (onSelectCallback) {
                            onSelectCallback(selectedFile);
                        }
                        isOpen = false;
                        ImGui::CloseCurrentPopup();
                        break;
                    }
                }
            }
        } catch (...) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error reading directory.");
        }

        ImGui::EndChild();

        ImGui::Separator();

        // Footer Actions
        ImGui::Text("Selected: %s", selectedFile.empty() ? "(None)" : selectedFile.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 150);

        if (ImGui::Button("Select", ImVec2(65, 0))) {
            if (!selectedFile.empty()) {
                if (onSelectCallback) {
                    onSelectCallback(selectedFile);
                }
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(65, 0))) {
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
