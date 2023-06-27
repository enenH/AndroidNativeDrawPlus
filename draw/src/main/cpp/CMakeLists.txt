cmake_minimum_required(VERSION 3.22)

#[[使用opengl绘制，否则使用vulkan绘制]]
set(OPENGL_DRAW OFF)

project(DrawCore)

set(CMAKE_CXX_STANDARD 17)

set(FLAG "-static-libstdc++ -fexceptions -DVK_USE_PLATFORM_ANDROID_KHR")

if (OPENGL_DRAW)
    add_definitions(-DOPENGL_DRAW)
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FLAG "-flto -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections -O3 -DNDEBUG -fno-stack-protector -Wno-builtin-macro-redefined -U__FILE__ -D__FILE__='\"\"'" "")

    set(LINKER_FLAGS "-flto -O3 -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
    set(CMAKE_SHARED_LINKER_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
else ()
    set(FLAG "-O0 -g")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/outputs/)

include_directories(
        imgui/
        imgui/backends/
        vulkan/
        Opengl/
        include/)


if (OPENGL_DRAW)
    set(SRC_LIST
            imgui/backends/imgui_impl_opengl3.cpp)
else ()
    set(SRC_LIST
            imgui/backends/imgui_impl_vulkan.cpp)
endif ()

add_library(libimgui STATIC
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        ${SRC_LIST}
        imgui/backends/imgui_impl_android.cpp)

if (OPENGL_DRAW)
    set(SRC_LIST
            Opengl/OpenglUtils.cpp)
else ()
    set(SRC_LIST
            vulkan/VulkanUtils.cpp
            vulkan/vulkan_wrapper.cpp)
endif ()

add_executable(entry
        main.cpp
        include/TouchHelperA.cpp
        include/jenv.cpp
        ${SRC_LIST})

target_link_libraries(entry
        libimgui
        GLESv3
        EGL
        z
        android
        log)