cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0048 NEW)

project(MERuntimeSource VERSION 1.0.0 DESCRIPTION "MANIFOLDEngine Runtime Source")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(vendor/MECore EXCLUDE_FROM_ALL)
add_subdirectory(vendor/vfspp EXCLUDE_FROM_ALL)

# IMGUI
set(imgui_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imconfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_internal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imstb_rectpack.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imstb_textedit.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imstb_truetype.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_sdl3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_sdl3.h)

add_library(imgui STATIC ${imgui_srcs})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
add_library(MERuntime SHARED)
target_sources(MERuntime PRIVATE ${SOURCES})
target_include_directories(MERuntime
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

MECore_configure(MERuntime)
target_link_libraries(MERuntime PRIVATE Threads::Threads Vulkan::Vulkan vfspp imgui)

add_library(ME::Runtime ALIAS MERuntime)
