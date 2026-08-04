#include "Application.hpp"
#include "raylib/raylib.h"

class TextureViewportTestingApp : public Application {
private:
    int perlinSeed = 10;
    int cellularSeed = 50;

protected:
    void Init() override {
        // 1. Register Procedural Perlin Noise Texture Viewport (Reloads via Callback on Ctrl+R)
        Image initialPerlinImg = GenImagePerlinNoise(256, 256, perlinSeed, perlinSeed, 4.0f);
        Texture2D initialPerlinTex = LoadTextureFromImage(initialPerlinImg);
        UnloadImage(initialPerlinImg);

        AddTextureViewport(
            initialPerlinTex,
            "Perlin Noise Texture",
            [this]() -> Texture2D {
                perlinSeed += 15;
                Image img = GenImagePerlinNoise(256, 256, perlinSeed, perlinSeed, 4.0f);
                Texture2D tex = LoadTextureFromImage(img);
                UnloadImage(img);
                return tex;
            },
            true, // ownsTexture
            true  // canClose
        );

        // 2. Register Procedural Cellular Noise Texture Viewport (Reloads via Callback on Ctrl+R)
        Image initialCellImg = GenImageCellular(256, 256, cellularSeed);
        Texture2D initialCellTex = LoadTextureFromImage(initialCellImg);
        UnloadImage(initialCellImg);

        AddTextureViewport(
            initialCellTex,
            "Cellular Noise Texture",
            [this]() -> Texture2D {
                cellularSeed += 25;
                Image img = GenImageCellular(256, 256, cellularSeed);
                Texture2D tex = LoadTextureFromImage(img);
                UnloadImage(img);
                return tex;
            },
            true, // ownsTexture
            true  // canClose
        );

        // 3. Register Disk File Texture Viewport (Empty initial path for disk image loading)
        AddTextureViewport("");
    }

    void Update(float deltaTime) override {
    }

    void Shutdown() override {
    }

public:
    TextureViewportTestingApp() : Application(1280, 720, "Texture Viewport Testing Application") {}
};

int main() {
    TextureViewportTestingApp app;
    app.Run();
    return 0;
}
