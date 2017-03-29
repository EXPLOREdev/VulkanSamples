/*
 * LunarGravity
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

#define APPLICATION_NAME "Lunar Gravity Demo"
#define APPLICATION_VERSION 1

int main(int argc, char *argv[]) {
    bool fullscreen = false;
    bool validate = false;
    bool enable_popups = true;
    LgLogLevel log_level = LG_LOG_ERROR;
    uint32_t win_width = 320;
    uint32_t win_height = 240;
    bool print_usage = false;
    for (int32_t i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][1] != '-') {
            print_usage = true;
            break;
        }
        if (!strcmp(&argv[i][2], "validate")) {
            validate = true;
        } else if (!strcmp(&argv[i][2], "fullscreen")) {
            fullscreen = true;
        } else if (!strcmp(&argv[i][2], "width")) {
            if (argc >= i + 1) {
                win_width = atoi(argv[++i]);
            } else {
                print_usage = true;
            }
        } else if (!strcmp(&argv[i][2], "height")) {
            if (argc >= i + 1) {
                win_height = atoi(argv[++i]);
            } else {
                print_usage = true;
            }
        } else if (!strcmp(&argv[i][2], "nopopups")) {
            enable_popups = false;
        } else if (!strcmp(&argv[i][2], "loglevel")) {
            if (argc >= i + 1) {
                if (!strcmp(argv[++i], "warn")) {
                    log_level = LG_LOG_WARN_ERROR;
                } else if (!strcmp(argv[i], "info")) {
                    log_level = LG_LOG_INFO_WARN_ERROR;
                } else if (!strcmp(argv[i], "all")) {
                    log_level = LG_LOG_ALL;
                } else {
                    print_usage = true;
                }
            } else {
                print_usage = true;
            }
        }
    }

    if (print_usage) {
        std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl
                  << "\t[OPTIONS]" << std::endl
                  << "\t\t--fullscreen\t\t\tEnable fullscreen render" << std::endl
                  << "\t\t--validate\t\t\tEnable validation" << std::endl
                  << "\t\t--loglevel [warn, info, all]\tEnable logging of provided level and above." << std::endl
                  << "\t\t--nopopups\t\t\tDisable warning/error pop-ups on Windows" << std::endl
                  << "\t\t--height val\t\t\tSet window height to val" << std::endl
                  << "\t\t--width val\t\t\tSet window width to val" << std::endl;
        return -1;
    }

    LgLogger &logger = LgLogger::getInstance();
    logger.SetLogLevel(log_level);
    logger.TogglePopups(enable_popups);
    LgWindow window(win_width, win_height, fullscreen);
    LgGraphicsEngine engine(APPLICATION_NAME, APPLICATION_VERSION, validate, window);
    return 0;
}
