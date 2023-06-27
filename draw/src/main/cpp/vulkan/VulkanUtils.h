#pragma once

#include <cstring>
#include "vulkan_wrapper.h"
#include "imgui.h"

// A struct to manage data related to one image in vulkan
struct MyTextureData {
    VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
    int Width;
    int Height;
    int Channels;

    // Need to keep track of these to properly cleanup
    VkImageView ImageView;
    VkImage Image;
    VkDeviceMemory ImageMemory;
    VkSampler Sampler;
    VkBuffer UploadBuffer;
    VkDeviceMemory UploadBufferMemory;

    MyTextureData() { memset(this, 0, sizeof(*this)); }
};

bool LoadTextureFromFile(const char *filename, MyTextureData *tex_data);

bool LoadTextureFromMemory(const void *filedata, int len, MyTextureData *tex_data);

void RemoveTexture(MyTextureData *tex_data);

void SetupVulkan();

void SetupVulkanWindow(ANativeWindow *window, int width, int height);

void UploadFonts();

void SwapChainRebuild(int w, int h);

void FrameRender(ImDrawData *draw_data);

void FramePresent();

void DeviceWait();

void CleanupVulkanWindow();

void CleanupVulkan();