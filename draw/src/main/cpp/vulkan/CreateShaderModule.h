/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef TUTORIAL06_TEXTURE_CREATESHADERMODULE_H
#define TUTORIAL06_TEXTURE_CREATESHADERMODULE_H

#include <vulkan_wrapper.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
/*
 * buildShaderFromFile()
 *   Create a Vulkan shader module from the given glsl shader file
 *   Input shader is compiled with shaderc (https://github.com/google/shaderc)
 *   prebuilt binary on github (https://github.com/ggfan/shaderc/release)
 *
 *   The pre-built shaderc lib is packed as CDep format of:
 *      https://github.com/google/cdep
 *   Refer to full documentation from the above homepage
 *
 *   feedback for CDep is very welcome to the https://github.com/google/cdep
 * Input:
 *     appInfo:   android_app, from which get AAssertManager*
 *     filePaht:  shader file full name with path inside APK/assets
 *     type:      borrowed VK's shader type to indicate which glsl shader it is
 *     vkDevice:  Vulkan logical device
 * Output:
 *     shaderOut:  built shader module return to caller
 * Return:
 *     VK_SUCCESS: shader module is at shaderOut
 *     Others:  error happened, no shader module created at all
 */

VkResult buildShaderFromFile(
    android_app* appInfo,
    const char* filePath,
    VkShaderStageFlagBits type,
    VkDevice vkDevice,
    VkShaderModule* shaderOut);

#endif // TUTORIAL06_TEXTURE_CREATESHADERMODULE_H
