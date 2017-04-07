/*
 * LunarGravity - lgwindow.cpp
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

#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>

#include "lglogger.hpp"
#include "lgwindow.hpp"
#include "lggfxengine.hpp"


#ifdef VK_USE_PLATFORM_WIN32_KHR
#pragma warning(push)
#pragma warning(disable: 4996) // Disable warning on using strncpy for portability
#endif // VK_USE_PLATFORM_WIN32_KHR

LgWindow::LgWindow(const char *win_name, const uint32_t width, const uint32_t height, bool fullscreen) {
    m_width = width;
    m_height = height;
    m_fullscreen = fullscreen;
    m_vk_surface = VK_NULL_HANDLE;
    m_win_name[0] = '\0';
    if (nullptr != win_name) {
        size_t name_length = strlen(win_name);
        if (name_length < 99) {
            strncpy(m_win_name, win_name, name_length);
            m_win_name[name_length + 1] = '\0';
        } else {
            strncpy(m_win_name, win_name, 99);
            m_win_name[99] = '\0';
        }
    }
}

LgWindow::~LgWindow() {
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
#pragma warning(pop)
#endif // VK_USE_PLATFORM_WIN32_KHR
