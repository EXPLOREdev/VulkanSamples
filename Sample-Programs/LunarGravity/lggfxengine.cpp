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

#define ENGINE_NAME "Lunar Gravity Graphics Engine"
#define ENGINE_VERSION 1

LgGraphicsEngine::LgGraphicsEngine(const std::string &app_name, uint16_t app_version, bool validate, LgWindow &window) {
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
        error_msg += count;
        logger.LogError(error_msg);
        return;
    }

    extension_properties.resize(count);
    vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extension_properties.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgWindow::QueryWindowSystem failed to query "
                   "vkEnumerateInstanceExtensionProperties with count ";
        error_msg += count;
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return;
    }

    enable_extension_count = count;
    if (!window.QueryWindowSystem(this, extension_properties, enable_extension_count, extensions_to_enable)) {
        logger.LogError("Failed LgWindow::QueryWindowSystem");
        exit(-1);
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
        logger.LogError("Cannot find a compatible Vulkan ICD");
        exit(-1);
    } else if (vk_result) {
        std::string error_msg = "Error: Unknown error ";
        error_msg += vk_result;
        error_msg += " encountered while attempting to create instance";
        logger.LogError(error_msg);
        exit(-1);
    }
 }
 
LgGraphicsEngine::~LgGraphicsEngine() {
    vkDestroyInstance(m_vk_inst, NULL);
}