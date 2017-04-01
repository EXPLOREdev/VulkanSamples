/*
 * LunarGravity - lgwin32window.hpp
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

#pragma once

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "lgwindow.hpp"

class LgWin32Window : public LgWindow {
    public:

        // Create a protected constructor
        LgWin32Window(const char *win_name, const uint32_t width, const uint32_t height, bool fullscreen);

        // We don't want any copy constructors
        LgWin32Window(const LgWin32Window &window) = delete;
        LgWin32Window &operator=(const LgWin32Window &window) = delete;

        // Make the destructor public
        virtual ~LgWin32Window();

        virtual bool CreateGfxWindow(VkInstance &instance);

    protected:
 
    private:
        HINSTANCE m_connection;
        HWND m_window;
        POINT m_minsize;
};

#endif // VK_USE_PLATFORM_WIN32_KHR