/*
 * LunarGravity - lggfxengine.cpp
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
#include <cstring>

#include "lggfxengine.hpp"
#include "lgwindow.hpp"

#ifdef _WIN32
//#include <windows.h>
#elif defined __ANDROID__
#else
#include <stdlib.h>
#endif

LgGraphicsEngine::LgGraphicsEngine(const std::string &app_name, uint16_t app_version, bool validate, LgWindow *window) {
    m_quit = false;
    m_window = window;
}

LgGraphicsEngine::~LgGraphicsEngine() {
}

LgSystemBatteryStatus LgGraphicsEngine::SystemBatteryStatus(void) {
    LgSystemBatteryStatus cur_status = LG_BATTERY_STATUS_NONE;

#ifdef _WIN32
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        switch (status.BatteryFlag) {
            default:
                break;
            case 0:
                cur_status = LG_BATTERY_STATUS_DISCHARGING_MID;
                break;
            case 1:
                cur_status = LG_BATTERY_STATUS_DISCHARGING_HIGH;
                break;
            case 2:
                cur_status = LG_BATTERY_STATUS_DISCHARGING_LOW;
                break;
            case 4:
                cur_status = LG_BATTERY_STATUS_DISCHARGING_CRITICAL;
                break;
            case 8:
                cur_status = LG_BATTERY_STATUS_CHARGING;
                break;
        }
    }

#elif defined __ANDROID__

#error "No Android support!"

#else

    // Check Linux battery level using ACPI:
    //  acpi -b output:
    //      Battery 0: Full, 100%
    //      Battery 0: Discharging, 95%, 10:32:44 remaining
    //      Battery 0: Charging, 94%, rate information unavailable
    const char battery_check_string[] = "acpi -b | awk -F'[, ]' '{ print $3 }'";
    const char power_level_string[] = "acpi -b | grep -P -o '[0-9]+(?=%)'";
    char result[256];
    FILE *fp = popen(battery_check_string, "r");
    if (fp != NULL) {
        bool discharging = false;
        if (fgets(result, sizeof(result) - 1, fp) != NULL) {
            if (NULL != strstr(result, "Charging")) {
                cur_status = LG_BATTERY_STATUS_CHARGING;
            } else if (NULL != strstr(result, "Discharging")) {
                discharging = true;
            }
        }
        pclose(fp);
        if (discharging) {
            fp = popen(power_level_string, "r");
            if (fp != NULL) {
                if (fgets(result, sizeof(result) - 1, fp) != NULL) {
                    int32_t battery_perc = atoi(result);
                    if (battery_perc > 66) {
                        cur_status = LG_BATTERY_STATUS_DISCHARGING_HIGH;
                    } else if (battery_perc > 33) {
                        cur_status = LG_BATTERY_STATUS_DISCHARGING_MID;
                    } else if (battery_perc > 5) {
                        cur_status = LG_BATTERY_STATUS_DISCHARGING_LOW;
                    } else {
                        cur_status = LG_BATTERY_STATUS_DISCHARGING_CRITICAL;
                    }
                }
                pclose(fp);
            }
        }
    }

#endif

    return cur_status;
}

void LgGraphicsEngine::Loop(void) {
    while (!m_quit) {
#if 0 // TODO : Brainpain
        demo_draw(demo);
        demo->curFrame++;
        if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount)
            demo->quit = true;
#endif
    }
}
