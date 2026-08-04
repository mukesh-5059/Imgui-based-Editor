#pragma once
#include "imgui/imgui.h"
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

class FilePicker {
private:
    std::filesystem::path currentPath;
    std::string selectedFile;
    std::vector<std::string> allowedExtensions;
    bool isOpen;
    std::string dialogTitle;
    std::function<void(const std::string&)> onSelectCallback;

    FilePicker();

public:
    static FilePicker& Get();

    void Open(const std::string& title = "Select File",
              const std::string& initialDir = "",
              const std::vector<std::string>& extensions = { ".png", ".jpg", ".jpeg", ".tga", ".bmp" },
              std::function<void(const std::string&)> onSelect = nullptr);

    void Draw();
    bool IsOpen() const { return isOpen; }
    void Close() { isOpen = false; }
};
