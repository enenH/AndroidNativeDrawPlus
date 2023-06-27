#include "OpenglUtils.h"
#include "imgui_impl_opengl3.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

static EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_EglSurface = EGL_NO_SURFACE;
static EGLContext g_EglContext = EGL_NO_CONTEXT;

void SetupOpengl(ANativeWindow *window) {
    const EGLint egl_attributes[] = {EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
                                     EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 16,
                                     EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE,
                                     EGL_WINDOW_BIT, EGL_NONE};
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_EglDisplay, nullptr, nullptr);

    EGLint num_configs = 0;
    eglChooseConfig(g_EglDisplay, egl_attributes, nullptr, 0, &num_configs);

    EGLConfig egl_config;
    eglChooseConfig(g_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
    EGLint egl_format;
    eglGetConfigAttrib(g_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
    ANativeWindow_setBuffersGeometry(window, 0, 0, egl_format);

    const EGLint egl_context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    g_EglContext = eglCreateContext(g_EglDisplay, egl_config, EGL_NO_CONTEXT,
                                    egl_context_attributes);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, egl_config, window, nullptr);
    eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void OpenglRender(ImDrawData *draw_data) {
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

void CleanupOpengl() {
    eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(g_EglDisplay, g_EglContext);
    eglDestroySurface(g_EglDisplay, g_EglSurface);
    eglTerminate(g_EglDisplay);
}

static void LoadTexture(void *image_data, MyTextureData *tex_data) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data->Width, tex_data->Height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    tex_data->DS = (ImTextureID) (intptr_t) textureId;
}

bool LoadTextureFromMemory(const void *filedata, int len, MyTextureData *tex_data) {
    tex_data->Channels = 4;
    unsigned char *image_data = stbi_load_from_memory((const stbi_uc *) filedata, len, &tex_data->Width,
                                                      &tex_data->Height, nullptr, tex_data->Channels);
    if (image_data == nullptr)
        return false;
    LoadTexture(image_data, tex_data);

    // Release image memory using stb
    stbi_image_free(image_data);
    return true;
}

// Helper function to load an image with common settings and return a MyTextureData with a VkDescriptorSet as a sort of Vulkan pointer
bool LoadTextureFromFile(const char *filename, MyTextureData *tex_data) {
    // Specifying 4 channels forces stb to load the image in RGBA which is an easy format for Vulkan
    tex_data->Channels = 4;
    unsigned char *image_data = stbi_load(filename, &tex_data->Width, &tex_data->Height, 0, tex_data->Channels);
    if (image_data == nullptr)
        return false;

    LoadTexture(image_data, tex_data);

    // Release image memory using stb
    stbi_image_free(image_data);
    return true;
}

// Helper function to cleanup an image loaded with LoadTextureFromFile
void RemoveTexture(MyTextureData *tex_data) {
    auto textureId = (GLuint) (intptr_t) tex_data->DS;
    glDeleteTextures(1, &textureId);
}
