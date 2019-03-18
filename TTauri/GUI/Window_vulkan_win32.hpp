#pragma once

#include "Window_vulkan.hpp"

#include <unordered_map>

#include <Windows.h>

namespace TTauri {
namespace GUI {

class Window_vulkan_win32 : public Window_vulkan {
public:
    HWND win32Window = nullptr;

    Window_vulkan_win32(std::shared_ptr<Delegate> delegate, const std::string &title);
    ~Window_vulkan_win32();

    vk::SurfaceKHR Window_vulkan_win32::createWindow(const std::string &title);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void createWindowClass();

    static const wchar_t *win32WindowClassName;
    static WNDCLASS win32WindowClass;
    static bool win32WindowClassIsRegistered;
    static std::unordered_map<std::uintptr_t, Window_vulkan_win32 *> win32WindowMap;
    static bool firstWindowHasBeenOpened;

private:
    static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
}}