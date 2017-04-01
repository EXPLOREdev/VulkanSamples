/*
 * LunarGravity - lgwin32window.cpp
 *
 * Copyright (C) 2017 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include <iostream>

#include "lgwin32window.hpp"
#include "lglogger.hpp"


LgWin32Window::LgWin32Window(const char *win_name, const uint32_t width, const uint32_t height,
                             bool fullscreen) :
    LgWindow(win_name, width, height, fullscreen) {
}

LgWin32Window::~LgWin32Window() {
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(validation_error);
        break;
    case WM_PAINT:
        // The validation callback calls MessageBox which can generate paint
        // events - don't make more Vulkan calls if we got here from the
        // callback
        if (!in_callback) {
            demo_run(&demo);
        }
        break;
    case WM_GETMINMAXINFO:     // set window's minimum size
        ((MINMAXINFO*)lParam)->ptMinTrackSize = demo.minsize;
        return 0;
    case WM_SIZE:
        // Resize the application to the new window size, except when
        // it was minimized. Vulkan doesn't support images or swapchains
        // with width=0 and height=0.
        if (wParam != SIZE_MINIMIZED) {
            demo.width = lParam & 0xffff;
            demo.height = (lParam & 0xffff0000) >> 16;
            demo_resize(&demo);
        }
        break;
    default:
        break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

bool LgWin32Window::CreateGfxWindow(VkInstance &instance) {
    LgLogger &logger = LgLogger::getInstance();
    RECT wr = {0, 0, m_width, m_height};

    WNDCLASSEX win_class_ex = {};
    win_class_ex.cbSize = sizeof(WNDCLASSEX);
    win_class_ex.style = CS_HREDRAW | CS_VREDRAW;
    win_class_ex.lpfnWndProc = WndProc;
    win_class_ex.cbClsExtra = 0;
    win_class_ex.cbWndExtra = 0;
    win_class_ex.hInstance = m_connection; // hInstance
    win_class_ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class_ex.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class_ex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class_ex.lpszMenuName = NULL;
    win_class_ex.lpszClassName = m_win_name;
    win_class_ex.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

    if (!RegisterClassEx(&win_class)) {
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class:
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    m_window = CreateWindowEx(0, m_win_name, m_win_name,
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
                              100, 100, wr.right - wr.left,
                              wr.bottom - wr.top, nullptr, nullptr,
                              m_connection, nullptr);
    if (!m_window) {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }
    // Window client area size must be at least 1 pixel high, to prevent crash.
    m_minsize.x = GetSystemMetrics(SM_CXMINTRACK);
    m_minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;


lt vk_result = vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr, &m_vk_surface);
    if (VK_SUCCESS != vk_result) {
        std::string error_msg = "LgWaylandWindow::CreateGfxWindow - vkCreateWaylandSurfaceKHR failed "
                                "with error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return false;
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR
