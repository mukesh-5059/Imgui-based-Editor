#pragma once
#include "raylib/raylib.h"
#include <vector>
#include <string>
#include <functional>

struct TextureViewport {
    int id = 0;
    std::string name;
    char filePath[512] = "";
    std::string loadedPath;
    Texture2D texture = { 0 };
    bool isLoaded = false;
    bool open = true;
    bool canClose = true;
    bool ownsTexture = true;
    std::function<Texture2D()> reloadCallback = nullptr;
};

struct SceneViewport {
    int id = 0;
    std::string name;
    RenderTexture2D renderTexture = { 0 };
    bool isLoaded = false;
    bool open = true;
    bool canClose = true;
    void* userData = nullptr;

    std::function<void(SceneViewport&)> sceneDrawCallback = nullptr;
    std::function<void(SceneViewport&)> inspectorDrawCallback = nullptr;
    std::function<void(SceneViewport&, Vector2 normPos)> clickCallback = nullptr;
};

class Application {
protected:
    int width;
    int height;
    const char* title;
    bool running;
    
    int m_targetFps;
    float m_frameTimeHistory[100];
    int m_lastTargetFps;
    int m_frameTimeIndex;

    RenderTexture2D sceneRenderTexture;
    std::vector<TextureViewport> textureViewports;
    int nextViewportId;

    // Multiple Scene Viewports & Workspaces
    std::vector<SceneViewport> sceneViewports;
    int activeSceneViewportIndex;
    int nextSceneViewportId;

    bool is3DViewportHovered;
    Vector2 viewportMouseNorm;

    float inspectorWidth;
    float consoleHeight;

    bool showConsole;
    bool showTableVisualizer;
    bool showInspector;

    void performanceGui();
    void renderResolutionGui();
    void SetRenderResolution(int newWidth, int newHeight);
    void editorGui();

    // Texture Viewport API
    int AddTextureViewport(const std::string& initialPath = "");
    int AddTextureViewport(Texture2D texture, const std::string& name = "", std::function<Texture2D()> reloadCallback = nullptr, bool ownsTexture = false, bool canClose = true);
    void RemoveTextureViewport(int index);
    void SetTextureViewportCallback(int viewportId, std::function<Texture2D()> reloadCallback);
    void SetTextureViewportTexture(int viewportId, Texture2D texture, bool ownsTexture = false);
    void ReloadTextureViewport(int viewportId);
    void ReloadTextureViewport(TextureViewport& vp);
    TextureViewport* GetTextureViewport(int viewportId);

    // Scene Viewport API
    int AddSceneViewport(const std::string& name = "Scene Viewport",
                         std::function<void(SceneViewport&)> sceneDraw = nullptr,
                         std::function<void(SceneViewport&)> inspectorDraw = nullptr,
                         std::function<void(SceneViewport&, Vector2 normPos)> onClick = nullptr,
                         bool canClose = true,
                         void* userData = nullptr);
    void RemoveSceneViewport(int index);
    SceneViewport* GetActiveSceneViewport();
    SceneViewport* GetSceneViewport(int viewportId);

    virtual void Init() {}
    virtual void Update(float deltaTime) {}
    virtual void SceneDraw() {}
    virtual void DrawInspectorUI() {}
    virtual void On3DViewportClicked(Vector2 mouseNormInViewport) {}
    virtual void Shutdown() {}

public:
    Application(int width = 1280, int height = 720, const char* title = "WorldBuilder");
    virtual ~Application();

    void Run();

    void EnableConsole(bool enable) { showConsole = enable; }
    void EnableTableVisualizer(bool enable) { showTableVisualizer = enable; }
    void EnableInspector(bool enable) { showInspector = enable; }
};
