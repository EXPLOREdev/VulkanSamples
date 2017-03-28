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
#include <vector>
#include <cstring>

#include "lglogger.hpp"
#include "lggfxengine.hpp"
#include "lgwindow.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined __ANDROID__
#else
#include <stdlib.h>
#endif

#define ENGINE_NAME "Lunar Gravity Graphics Engine"
#define ENGINE_VERSION 1

LgGraphicsEngine::LgGraphicsEngine(const std::string &app_name, uint16_t app_version, bool validate, LgWindow &window) {
    if (!InitVulkan(app_name, app_version, validate, window)) {
        exit(-1);
    }
}

LgGraphicsEngine::~LgGraphicsEngine() {
    vkDestroyInstance(m_vk_inst, NULL);
}

bool LgGraphicsEngine::InitVulkan(const std::string &app_name, uint16_t app_version, bool validate, LgWindow &window) {
    VkApplicationInfo vk_app_info = {};
    VkInstanceCreateInfo vk_inst_create_info = {};
    VkResult vk_result;
    uint32_t count = 0;
    std::vector<VkLayerProperties> layer_properties;
    std::vector<VkExtensionProperties> extension_properties;
    uint32_t enable_extension_count = 0;
    const char *extensions_to_enable[VK_MAX_EXTENSION_NAME_SIZE];
    uint32_t enable_layer_count = 0;
    const char *layers_to_enable[VK_MAX_EXTENSION_NAME_SIZE];
    LgLogger &logger = LgLogger::getInstance();
    std::vector<VkPhysicalDevice> physical_devices;

    memset(extensions_to_enable, 0, sizeof(extensions_to_enable));
    memset(layers_to_enable, 0, sizeof(layers_to_enable));

    // Define this application info first
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pNext = nullptr;
    vk_app_info.pApplicationName = app_name.c_str();
    vk_app_info.applicationVersion = app_version;
    vk_app_info.pEngineName = ENGINE_NAME;
    vk_app_info.engineVersion = ENGINE_VERSION;
    vk_app_info.apiVersion = VK_API_VERSION_1_0;

    // Define Vulkan Instance Create Info
    vk_inst_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_inst_create_info.pNext = nullptr;
    vk_inst_create_info.flags = 0;
    vk_inst_create_info.pApplicationInfo = &vk_app_info;
    vk_inst_create_info.enabledExtensionCount = 0;
    vk_inst_create_info.ppEnabledExtensionNames = nullptr;
    vk_inst_create_info.enabledLayerCount = 0;
    vk_inst_create_info.ppEnabledLayerNames = nullptr;

    // If user wants to validate, check to see if we can enable it.
    if (validate) {
        vk_result = vkEnumerateInstanceLayerProperties(&count, nullptr);
        if (vk_result == VK_SUCCESS && count > 0) {
            layer_properties.resize(count);
            vk_result = vkEnumerateInstanceLayerProperties(&count, layer_properties.data());
            if (vk_result == VK_SUCCESS && count > 0) {
                for (uint32_t layer = 0; layer < count; layer++) {
                    if (!strcmp(layer_properties[layer].layerName, "VK_LAYER_LUNARG_standard_validation")) {
                        m_validation_enabled = true;
                        layers_to_enable[enable_layer_count++] = "VK_LAYER_LUNARG_standard_validation";
                        logger.LogInfo("Found standard validation layer");
                    }
                }
            }
        }
    }

    vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgWindow::QueryWindowSystem failed to query "
                  "vkEnumerateInstanceExtensionProperties first time with error ";
        error_msg += vk_result;
        error_msg += " and count ";
        error_msg += std::to_string(count);
        logger.LogError(error_msg);
        return false;
    }

    extension_properties.resize(count);
    vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extension_properties.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgWindow::QueryWindowSystem failed to query "
                   "vkEnumerateInstanceExtensionProperties with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return false;
    }

    enable_extension_count = count;
    if (!window.QueryWindowSystem(this, extension_properties, enable_extension_count, extensions_to_enable)) {
        logger.LogError("Failed LgWindow::QueryWindowSystem");
        return false;
    }

    for (uint32_t ext = 0; ext < count; ext++) {
        if (!strcmp(extension_properties[ext].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            m_debug_enabled = true;
            extensions_to_enable[enable_extension_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            logger.LogInfo("Found debug report extension in Instance Extension list");
        }
    }

    vk_inst_create_info.enabledExtensionCount = enable_extension_count;
    vk_inst_create_info.ppEnabledExtensionNames = (const char *const *)extensions_to_enable;
    vk_inst_create_info.enabledLayerCount = enable_layer_count;
    vk_inst_create_info.ppEnabledLayerNames = (const char *const *)layers_to_enable;

    vk_result = vkCreateInstance(&vk_inst_create_info, nullptr, &m_vk_inst);
    if (vk_result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        logger.LogError("LgWindow::QueryWindowSystem failed vkCreateInstance could not find a "
                        "compatible Vulkan ICD");
        return false;
    } else if (vk_result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        logger.LogError("LgWindow::QueryWindowSystem failed vkCreateInstance could not find "
                        "one or more extensions");
        return false;
    } else if (vk_result) {
        std::string error_msg = "LgWindow::QueryWindowSystem failed vkCreateInstance ";
        error_msg += vk_result;
        error_msg += " encountered while attempting to create instance";
        logger.LogError(error_msg);
        return false;
    }

    vk_result = vkEnumeratePhysicalDevices(m_vk_inst, &count, nullptr);
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgWindow::QueryWindowSystem failed to query "
                                "vkEnumeratePhysicalDevices first time with error ";
        error_msg += vk_result;
        error_msg += " and count ";
        error_msg += std::to_string(count);
        logger.LogError(error_msg);
        return false;
    }
    physical_devices.resize(count);
    vk_result = vkEnumeratePhysicalDevices(m_vk_inst, &count, physical_devices.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgWindow::QueryWindowSystem failed to query "
                                "vkEnumeratePhysicalDevices with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return false;
    }

    int32_t best_integrated_index = -1;
    int32_t best_discrete_index = -1;
    int32_t best_virtual_index = -1;
    std::vector<VkPhysicalDeviceProperties> phys_dev_props;
    phys_dev_props.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        vkGetPhysicalDeviceProperties(physical_devices[i], &phys_dev_props[i]);
        
        switch (phys_dev_props[i].deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                logger.LogInfo("Other device found");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                logger.LogInfo("Integrated GPU found");
                if (best_integrated_index != -1) {
                    if (CompareGpus(phys_dev_props[best_integrated_index],
                                    phys_dev_props[i])) {
                        best_integrated_index = i;
                    }
                } else {
                    best_integrated_index = i;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                logger.LogInfo("Discrete GPU found");
                if (best_discrete_index != -1) {
                    if (CompareGpus(phys_dev_props[best_discrete_index],
                                    phys_dev_props[i])) {
                        best_discrete_index = i;
                    }
                } else {
                    best_discrete_index = i;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                logger.LogInfo("Virtual GPU found");
                if (best_virtual_index != -1) {
                    if (CompareGpus(phys_dev_props[best_virtual_index],
                                    phys_dev_props[i])) {
                        best_virtual_index = i;
                    }
                } else {
                    best_virtual_index = i;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                logger.LogInfo("CPU found");
                break;
            default:
                break;
        }
    }

    // If we have the choice between discrete and integrated, look at the
    // battery status to help make the decision.  If running on battery, use
    // integrated.  Otherwise, choose discrete.
    if (best_discrete_index != -1 && best_integrated_index != -1) {
        LgSystemBatteryStatus battery_status = SystemBatteryStatus();
        switch (battery_status) {
            case LG_BATTERY_STATUS_NONE:
            case LG_BATTERY_STATUS_CHARGING:
                m_vk_phys_dev = physical_devices[best_discrete_index];
                break;
            default:
                m_vk_phys_dev = physical_devices[best_integrated_index];
                break;
        }
    // Otherwise, we have one or the other.
    } else if (best_discrete_index != -1) {
        m_vk_phys_dev = physical_devices[best_discrete_index];
    } else if (best_integrated_index != -1) {
        m_vk_phys_dev = physical_devices[best_integrated_index];
    } else if (best_virtual_index != -1) {
        m_vk_phys_dev = physical_devices[best_virtual_index];
    } else {
        logger.LogError("Failed to find a GPU of any kind");
        return false;
    }

    return true;
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
            case 4
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

int LgGraphicsEngine::CompareGpus(VkPhysicalDeviceProperties &gpu_0,
                                  VkPhysicalDeviceProperties &gpu_1) {
    int gpu_to_use = 1;
    bool determined = false;

    // For now, use discrete over integrated
    if (gpu_0.deviceType != gpu_1.deviceType) {
        if (gpu_0.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            gpu_to_use = 0;
            determined = true;
        } else if (gpu_1.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            gpu_to_use = 1;
            determined = true;
        } else if (gpu_0.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            gpu_to_use = 0;
            determined = true;
        } else if (gpu_1.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            gpu_to_use = 1;
            determined = true;
        }
    }

    // For now, use newest API version if we got this far.
    if (!determined) {
        uint16_t major_0 = VK_VERSION_MAJOR(gpu_0.apiVersion);
        uint16_t minor_0 = VK_VERSION_MINOR(gpu_0.apiVersion);
        uint16_t major_1 = VK_VERSION_MAJOR(gpu_1.apiVersion);
        uint16_t minor_1 = VK_VERSION_MINOR(gpu_1.apiVersion);

        if (major_0 != major_1) {
            if (major_0 > major_1) {
                gpu_to_use = 0;
            } else {
                gpu_to_use = 1;
            }
        } else {
            if (minor_0 > minor_1) {
                gpu_to_use = 0;
            } else {
                gpu_to_use = 1;
            }
        }
    }
    return gpu_to_use;
}