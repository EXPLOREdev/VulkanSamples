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

    m_instance = GetModuleHandle(NULL);
}

LgWin32Window::~LgWin32Window() {
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LgLogger &logger = LgLogger::getInstance();
    switch (uMsg) {
    case WM_CLOSE:
        logger.LogInfo("LgWin32Window::WndProc -Received close event");
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        // TODO: Brainpain - Do we need to handle this?
        break;
    case WM_GETMINMAXINFO:     // set window's minimum size
        // TODO: Brainpain - Do we need to handle this?
        //((MINMAXINFO*)lParam)->ptMinTrackSize = m_minsize;
        return 0;
    case WM_SIZE:
        // TODO: Brainpain - Post re-size message to app
        // Resize the application to the new window size, except when
        // it was minimized. Vulkan doesn't support images or swapchains
        // with width=0 and height=0.
        //if (wParam != SIZE_MINIMIZED) {
        //    demo.width = lParam & 0xffff;
        //    demo.height = (lParam & 0xffff0000) >> 16;
        //    demo_resize(&demo);
        //}
        break;
    default:
        break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

bool LgWin32Window::CreateGfxWindow(VkInstance &instance) {
    LgLogger &logger = LgLogger::getInstance();
    RECT wr = {0, 0, (LONG)m_width, (LONG)m_height};
    DWORD ext_win_style = WS_EX_APPWINDOW;
    DWORD win_style = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    WNDCLASSEX win_class_ex = {};

    win_class_ex.cbSize = sizeof(WNDCLASSEX);
    win_class_ex.style = CS_HREDRAW | CS_VREDRAW;
    win_class_ex.lpfnWndProc = WndProc;
    win_class_ex.cbClsExtra = 0;
    win_class_ex.cbWndExtra = 0;
    win_class_ex.hInstance = m_instance;
    win_class_ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class_ex.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class_ex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class_ex.lpszMenuName = NULL;
    win_class_ex.lpszClassName = m_win_name;
    win_class_ex.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    if (!RegisterClassEx(&win_class_ex)) {
        logger.LogError("LgWin32Window::CreateGfxWindow - Failed attempting"
                        "to Register window class!");
        return false;
    }

    if (m_fullscreen) {
        // Determine if we can go fullscreen with the provided settings.
        DEVMODE dev_mode_settings = {};
        dev_mode_settings.dmSize = sizeof(DEVMODE);
        dev_mode_settings.dmPelsWidth = m_width;
        dev_mode_settings.dmPelsHeight = m_height;
        dev_mode_settings.dmBitsPerPel = 32;
        dev_mode_settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
        if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings(&dev_mode_settings, CDS_FULLSCREEN))
        {
            if (IDYES == MessageBox(nullptr, "Failed fullscreen at provided resolution.  Go Windowed?", m_win_name,
                           MB_YESNO | MB_ICONEXCLAMATION))
            {
                // Fallback to windowed mode
                m_fullscreen = FALSE;
            } else {
                // Give up
                logger.LogError("LgWin32Window::CreateGfxWindow - Failed to create"
                                "fullscreen window at provided resolution.");
                return false;
            }
        }
    }

    if (m_fullscreen) {
        win_style |= WS_POPUP;
        ShowCursor(FALSE);
    } else {
        win_style |= WS_OVERLAPPEDWINDOW;
        ext_win_style |= WS_EX_WINDOWEDGE;
    }

    // Create window with the registered class:
    AdjustWindowRectEx(&wr, win_style, FALSE, ext_win_style);
    m_window = CreateWindowEx(ext_win_style, m_win_name, m_win_name, win_style,
                              100, 100, wr.right - wr.left,
                              wr.bottom - wr.top, nullptr, nullptr,
                              m_instance, nullptr);
    if (!m_window) {
        logger.LogError("LgWin32Window::CreateGfxWindow - Failed attempting"
                        "to create window class!");
        fflush(stdout);
        return false;
    }
    // Window client area size must be at least 1 pixel high, to prevent crash.
    m_minsize.x = GetSystemMetrics(SM_CXMINTRACK);
    m_minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;

    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = m_instance;
    createInfo.hwnd = m_window;
    VkResult vk_result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_vk_surface);
    if (VK_SUCCESS != vk_result) {
        std::string error_msg = "LgWin32Window::CreateGfxWindow - vkCreateWaylandSurfaceKHR failed "
                                "with error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return false;
    }

    return true;
}

bool LgWin32Window::CloseGfxWindow() {
    if (m_fullscreen) {
        ChangeDisplaySettings(nullptr, 0);
        ShowCursor(TRUE);
    }
    return true;
}


#endif // VK_USE_PLATFORM_WIN32_KHR
