#pragma once

#include "imgui.h"
#include <android/native_window.h>

// A struct to manage data related to one image in vulkan
struct MyTextureData {
    ImTextureID DS;         // Descriptor set: this is what you'll pass to Image()
    int Width;
    int Height;
    int Channels;

    MyTextureData() { memset(this, 0, sizeof(*this)); }
};

bool LoadTextureFromFile(const char *filename, MyTextureData *tex_data);

bool LoadTextureFromMemory(const void *filedata, int len, MyTextureData *tex_data);

void RemoveTexture(MyTextureData *tex_data);

void SetupOpengl(ANativeWindow *window);

void OpenglRender(ImDrawData *draw_data);

void CleanupOpengl();