#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <ctime>
#include "imgui_impl_android.h"
#include "TouchHelperA.h"
#include "Log.h"
#include "jenv.h"
#include "classes.h"

#if defined(USE_OPENGL)
#include "imgui_impl_opengl3.h"
#include "OpenglUtils.h"

#else

#include "imgui_impl_vulkan.h"
#include "VulkanUtils.h"

#endif

float screenWidth = 0;
float screenHeight = 0;
uint32_t orientation = 0;
bool other_touch = false;
bool flag = true;

JavaVM *g_jvm;
JNIEnv *g_env;
jclass entryClass;

static bool Android_LoadSystemFont(float size) {
    char path[64]{0};
    char *filename = nullptr;
    const char *fontPath[] = {
            "/system/fonts", "/system/font", "/data/fonts"
    };
    for (auto tmp: fontPath) {
        if (access(tmp, R_OK) == 0) {
            strcpy(path, tmp);
            filename = path + strlen(tmp);
            break;
        }
    }
    if (!filename) {
        return false;
    }
    *filename++ = '/';
    strcpy(filename, "NotoSansCJK-Regular.ttc");
    if (access(path, R_OK) != 0) {
        strcpy(filename, "NotoSerifCJK-Regular.ttc");
        if (access(path, R_OK) != 0) {
            return false;
        }
    }
    ImGuiIO &io = ImGui::GetIO();
    static ImVector<ImWchar> ranges;
    if (ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        constexpr ImWchar Ranges[]{
                0x0020, 0x00FF, // Basic Latin
                0x0100, 0x024F, // Latin Extended-A + Latin Extended-B
                0x0300, 0x03FF, // Combining Diacritical Marks + Greek/Coptic
                0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
                0x0600, 0x06FF, // Arabic
                0x0E00, 0x0E7F, // Thai
                0x2DE0, 0x2DFF, // Cyrillic Extended-A
                0x2000, 0x206F, // General Punctuation
                0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
                0x31F0, 0x31FF, // Katakana Phonetic Extensions
                0xFF00, 0xFFEF, // Half-width characters
                //0x4E00, 0x9FAF, // CJK Ideograms
                0xA640, 0xA69F, // Cyrillic Extended-B
                0x3131, 0x3163, // Korean alphabets
                //  0xAC00, 0xD7A3, // Korean characters
                0
        };
        builder.AddRanges(Ranges);
        builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        builder.BuildRanges(&ranges);
    }
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.SizePixels = size;
    config.GlyphRanges = ranges.Data;
    config.OversampleH = 1;
    return io.Fonts->AddFontFromFileTTF(path, 0, &config);
}

const char *getClipText() {
    seteuid(2000);
    //获取env
    JNIEnv *env = g_env;
    //g_jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    static jmethodID getClipText = nullptr;
    if (!getClipText) {
        getClipText = env->GetStaticMethodID(entryClass, "getClipText",
                                             "()Ljava/lang/String;");
    }
    auto jstr = (jstring) env->CallStaticObjectMethod(entryClass, getClipText);
    char *str = (char *) env->GetStringUTFChars(jstr, NULL);
    static char buff[1024];
    strncpy(buff, str, sizeof(buff) - 1);
    env->ReleaseStringUTFChars(jstr, str);
    seteuid(0);
    return buff;
}


void surfaceCreate(JNIEnv *env, jobject surface) {
    auto native_window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_acquire(native_window);
    float width, height;
    width = (float) ANativeWindow_getWidth(native_window);
    height = (float) ANativeWindow_getHeight(native_window);
    if (width > height) {
        screenWidth = width;
        screenHeight = height;
    } else {
        screenWidth = height;
        screenHeight = width;
    }
#if defined(USE_OPENGL)
    SetupOpengl(native_window);
#else
    InitVulkan();
    SetupVulkan();
    SetupVulkanWindow(native_window, (int) screenWidth, (int) screenWidth);
#endif
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = {screenWidth, screenWidth};
    io.FontGlobalScale = 1.3;
    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(3);
    style.WindowRounding = 2;
    ImGui_ImplAndroid_Init(native_window);

#if defined(USE_OPENGL)
    ImGui_ImplOpenGL3_Init("#version 300 es");
#endif
    Android_LoadSystemFont(26);
#ifndef USE_OPENGL
    UploadFonts();
#endif
    //触摸初始化
    Touch_Init(screenHeight, screenWidth, true);
    while (flag) {
#ifdef USE_OPENGL
        ImGui_ImplOpenGL3_NewFrame();
#else
        ImGui_ImplVulkan_NewFrame();
#endif
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

#if defined(USE_OPENGL)
        const char* name = "OpenGL";
#else
        const char *name = "Vulkan";
#endif
        ImGui::SetNextWindowSize({500, 500}, ImGuiCond_Once);
        if (ImGui::Begin(name, &flag)) {
            ImGui::Text("Hello, world!");
            static const char *text = "null";
            ImGui::Text("%s", text);
            if (ImGui::Button("copy")) {
                text = getClipText();
            }
        }
        ImGui::End();
        // Rendering
        ImGui::Render();
#ifdef USE_OPENGL
        OpenglRender(ImGui::GetDrawData());
#else
        FrameRender(ImGui::GetDrawData());
        FramePresent();
#endif
    }
    Touch_Close();
    // Cleanup
#ifdef USE_OPENGL
    ImGui_ImplOpenGL3_Shutdown();
#else
    DeviceWait();
    ImGui_ImplVulkan_Shutdown();
#endif

    ImGui::DestroyContext();
#ifdef USE_OPENGL
    CleanupOpengl();
#else
    CleanupVulkanWindow();
    CleanupVulkan();
#endif
    ANativeWindow_release(native_window);
    LOGD("stop");
}

void surfaceChanged(JNIEnv *env, jclass clazz, jint rotation) {
    orientation = rotation;
}

int main(int argc, char **argv) {
    JavaCTX ctx;
    if (init_java_env(&ctx, nullptr, 0) != JNI_OK) {
        printf("init java env failed");
        return -1;
    }
    g_jvm = ctx.vm;
    g_env = ctx.env;

    JNIEnv *env = g_env;
    // First, get the system classloader
    LOGD("get system classloader");
    auto clClass = env->FindClass("java/lang/ClassLoader");
    auto getSystemClassLoader = env->GetStaticMethodID(clClass, "getSystemClassLoader",
                                                       "()Ljava/lang/ClassLoader;");
    auto systemClassLoader = env->CallStaticObjectMethod(clClass, getSystemClassLoader);
    LOGD("systemClassLoader %p", systemClassLoader);
    // Assuming we have a valid mapped module, load it. This is similar to the approach used for
    // Dynamite modules in GmsCompat, except we can use InMemoryDexClassLoader directly instead of
    // tampering with DelegateLastClassLoader's DexPathList.
    LOGD("create buffer");
    auto buf = env->NewDirectByteBuffer((void *) classes, sizeof(classes));
    LOGD("create class loader");
    auto dexClClass = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    auto dexClInit = env->GetMethodID(dexClClass, "<init>",
                                      "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    auto dexCl = env->NewObject(dexClClass, dexClInit, buf, systemClassLoader);

    // Load the class
    LOGD("load class");
    auto loadClass = env->GetMethodID(clClass, "loadClass",
                                      "(Ljava/lang/String;)Ljava/lang/Class;");
    auto entryClassName = env->NewStringUTF("com.example.draw.Main");
    LOGD("entryClassName %p", entryClassName);
    auto entryClassObj = env->CallObjectMethod(dexCl, loadClass, entryClassName);
    LOGD("entryClassObj %p", entryClassObj);

    //call main
    entryClass = (jclass) env->NewGlobalRef(entryClassObj);
    /*auto entryInit = env->GetStaticMethodID(entryClass, "main", "([Ljava/lang/String;)V");
    env->CallStaticVoidMethod(entryClass, entryInit);*/
    //entryClass = env->FindClass("com/example/draw/Main");
    static JNINativeMethod methods[] = {
            // 三个参数分别为：java的函数名，函数返回属性（return xx）, jni函数名
            {"native_surfaceChanged", "(I)V", (void *) surfaceChanged},
    };

    jint methodSize = sizeof(methods) / sizeof(methods[0]);
    env->RegisterNatives(entryClass, methods, methodSize);
    //全机型防录屏截屏
    bool isHide = true;
    //黑屏
    bool isSecure = false;

    jmethodID methodId = env->GetStaticMethodID(entryClass, "getSurface",
                                                "(ZZ)Landroid/view/Surface;");
    jobject surface = env->CallStaticObjectMethod(entryClass, methodId, isHide, isSecure);
    surfaceCreate(env, surface);
    printf("stop");
    return 0;
};