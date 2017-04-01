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

#ifdef VK_USE_PLATFORM_XCB_KHR

#include <iostream>

#include "lgxcbwindow.hpp"
#include "lglogger.hpp"

LgXcbWindow::LgXcbWindow(const char *win_name, const uint32_t width, const uint32_t height, bool fullscreen) :
    LgWindow(win_name, width, height, fullscreen) {
    LgLogger &logger = LgLogger::getInstance();
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    m_connection = xcb_connect(nullptr, &scr);
    if (xcb_connection_has_error(m_connection) > 0) {
        logger.LogError("LgXcbWindow::LgXcbWindow Xcb Connection failed");
        exit(-1);
    }

    setup = xcb_get_setup(m_connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) {
        xcb_screen_next(&iter);
    }

    m_screen = iter.data;
    m_xcb_window = 0;
    m_atom_wm_delete_window = 0;
}

LgXcbWindow::~LgXcbWindow() {
    xcb_destroy_window(m_connection, m_xcb_window);
    xcb_disconnect(m_connection);
    free(m_atom_wm_delete_window);
}

bool LgXcbWindow::CreateGfxWindow(VkInstance &instance) {
    uint32_t value_mask, value_list[32];
    LgLogger &logger = LgLogger::getInstance();

    m_xcb_window = xcb_generate_id(m_connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = m_screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_xcb_window,
                      m_screen->root, 0, 0, m_width, m_height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, m_screen->root_visual,
                      value_mask, value_list);

    // Magic code that will send notification when window is destroyed
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(m_connection, cookie, 0);
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_connection, 0, 16, "WM_DELETE_WINDOW");
    m_atom_wm_delete_window = xcb_intern_atom_reply(m_connection, cookie2, 0);

    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_xcb_window,
                        (*reply).atom, 4, 32, 1, &(*m_atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(m_connection, m_xcb_window);

    // Force the x/y coordinates to 100,100 results are identical in consecutive
    // runs
    const uint32_t coords[] = { 100, 100 };
    xcb_configure_window(m_connection, m_xcb_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    VkXcbSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.connection = m_connection;
    createInfo.window = m_xcb_window;
    VkResult vk_result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &m_vk_surface);
    if (VK_SUCCESS != vk_result) {
        std::string error_msg = "LgXcbWindow::CreateGfxWindow - vkCreateXcbSurfaceKHR failed "
                                "with error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return false;
    }

    return true;
}

#if 0 // TODO: Brainpain

static void demo_handle_xcb_event(struct demo *demo,
                              const xcb_generic_event_t *event) {
    uint8_t event_code = event->response_type & 0x7f;
    switch (event_code) {
    case XCB_EXPOSE:
        // TODO: Resize window
        break;
    case XCB_CLIENT_MESSAGE:
        if ((*(xcb_client_message_event_t *)event).data.data32[0] ==
            (*demo->atom_wm_delete_window).atom) {
            demo->quit = true;
        }
        break;
    case XCB_KEY_RELEASE: {
        const xcb_key_release_event_t *key =
            (const xcb_key_release_event_t *)event;

        switch (key->detail) {
        case 0x9: // Escape
            demo->quit = true;
            break;
        case 0x71: // left arrow key
            demo->spin_angle -= demo->spin_increment;
            break;
        case 0x72: // right arrow key
            demo->spin_angle += demo->spin_increment;
            break;
        case 0x41: // space bar
            demo->pause = !demo->pause;
            break;
        }
    } break;
    case XCB_CONFIGURE_NOTIFY: {
        const xcb_configure_notify_event_t *cfg =
            (const xcb_configure_notify_event_t *)event;
        if ((demo->width != cfg->width) || (demo->height != cfg->height)) {
            demo->width = cfg->width;
            demo->height = cfg->height;
            demo_resize(demo);
        }
    } break;
    default:
        break;
    }
}

static void demo_run_xcb(struct demo *demo) {
    xcb_flush(demo->connection);

    while (!demo->quit) {
        xcb_generic_event_t *event;

        if (demo->pause) {
            event = xcb_wait_for_event(demo->connection);
        }
        else {
            event = xcb_poll_for_event(demo->connection);
        }
        while (event) {
            demo_handle_xcb_event(demo, event);
            free(event);
            event = xcb_poll_for_event(demo->connection);
        }

        demo_draw(demo);
        demo->curFrame++;
        if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount)
            demo->quit = true;
    }
}

#endif // Brainpain

#endif // VK_USE_PLATFORM_XCB_KHR