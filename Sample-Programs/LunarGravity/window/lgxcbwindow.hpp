/*
 * LunarGravity - lgxcbwindow.hpp
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

#ifdef VK_USE_PLATFORM_XCB_KHR

#include <X11/Xutil.h>

#include "lgwindow.hpp"

class LgXcbWindow : public LgWindow {
    public:

        // Create a protected constructor
        LgXcbWindow(const char *win_name, const uint32_t width, const uint32_t height, bool fullscreen);

        // We don't want any copy constructors
        LgXcbWindow(const LgXcbWindow &window) = delete;
        LgXcbWindow &operator=(const LgXcbWindow &window) = delete;

        // Make the destructor public
        virtual ~LgXcbWindow();

        virtual bool CreateGfxWindow(VkInstance &instance);

    protected:
 
    private:
        Display *m_display;
        xcb_connection_t *m_connection;
        xcb_screen_t *m_screen;
        xcb_window_t m_xcb_window;
        xcb_intern_atom_reply_t *m_atom_wm_delete_window;
};

#endif // VK_USE_PLATFORM_XCB_KHR
