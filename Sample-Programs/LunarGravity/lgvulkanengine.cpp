/*
 * LunarGravity - lgvulkanengine.cpp
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
#include <sstream>
#include <cstdlib>
#include <vector>
#include <cstring>

#include "vk_dispatch_table_helper.h"

#include "lglogger.hpp"
#include "lgvulkanengine.hpp"
#include "lgwindow.hpp"

#ifdef _WIN32
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined __ANDROID__
#else
#include <stdlib.h>
#endif

#define ENGINE_NAME "Lunar Gravity Graphics Engine"
#define ENGINE_VERSION 1

LgVulkanEngine::LgVulkanEngine(const std::string &app_name, uint16_t app_version, bool validate, LgWindow *window) :
    LgGraphicsEngine(app_name, app_version, validate, window) {
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
    uint32_t queue_family_count = 0;

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
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                  "vkEnumerateInstanceExtensionProperties first time with error ";
        error_msg += vk_result;
        error_msg += " and count ";
        error_msg += std::to_string(count);
        logger.LogError(error_msg);
        exit(-1);
    }

    extension_properties.resize(count);
    vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extension_properties.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                   "vkEnumerateInstanceExtensionProperties with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        exit(-1);
    }

    enable_extension_count = count;
    if (!QueryWindowSystem(extension_properties, enable_extension_count, extensions_to_enable)) {
        logger.LogError("Failed LgVulkanEngine::QueryWindowSystem");
        exit(-1);
    }

    for (uint32_t ext = 0; ext < count; ext++) {
        if (!strcmp(extension_properties[ext].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            m_debug_enabled = true;
            extensions_to_enable[enable_extension_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            logger.LogInfo("Found debug report extension in Instance Extension list");
        }
    }

    VkDebugReportCallbackCreateInfoEXT dbg_create_info = {};

    LgLogLevel level = logger.GetLogLevel();
    if (level > LG_LOG_DISABLE) {
        dbg_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        dbg_create_info.pNext = nullptr;
        dbg_create_info.pfnCallback = LoggerCallback;
        dbg_create_info.pUserData = this;
        switch (level) {
            case LG_LOG_ALL:
                dbg_create_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT;
            case LG_LOG_INFO_WARN_ERROR:
                dbg_create_info.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
            case LG_LOG_WARN_ERROR:
                dbg_create_info.flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                         VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            case LG_LOG_ERROR:
                dbg_create_info.flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
                break;
            default:
                break;
        }
    }

    vk_inst_create_info.enabledExtensionCount = enable_extension_count;
    vk_inst_create_info.ppEnabledExtensionNames = (const char *const *)extensions_to_enable;
    vk_inst_create_info.enabledLayerCount = enable_layer_count;
    vk_inst_create_info.ppEnabledLayerNames = (const char *const *)layers_to_enable;

    if (level > LG_LOG_DISABLE) {
        vk_inst_create_info.pNext = &dbg_create_info;
    }

    vk_result = vkCreateInstance(&vk_inst_create_info, nullptr, &m_vk_inst);
    if (vk_result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        logger.LogError("LgVulkanEngine::LgVulkanEngine failed vkCreateInstance could not find a "
                        "compatible Vulkan ICD");
        exit(-1);
    } else if (vk_result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        logger.LogError("LgVulkanEngine::LgVulkanEngine failed vkCreateInstance could not find "
                        "one or more extensions");
        exit(-1);
    } else if (vk_result) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed vkCreateInstance ";
        error_msg += vk_result;
        error_msg += " encountered while attempting to create instance";
        logger.LogError(error_msg);
        exit(-1);
    }

    // Hijack the layer dispatch table init and use it
    layer_init_instance_dispatch_table(m_vk_inst, &m_vk_inst_dispatch_table, vkGetInstanceProcAddr);

    if (level > LG_LOG_DISABLE) {
        vk_result = m_vk_inst_dispatch_table.CreateDebugReportCallbackEXT(m_vk_inst, &dbg_create_info, nullptr,
                                                                          &m_dbg_report_callback);
        if (vk_result != VK_SUCCESS) {
            std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed call to "
                                    "CreateDebugReportCallback with error";
            error_msg += vk_result;
            logger.LogError(error_msg);
            exit(-1);
        }
    }

    vk_result = vkEnumeratePhysicalDevices(m_vk_inst, &count, nullptr);
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                                "vkEnumeratePhysicalDevices first time with error ";
        error_msg += vk_result;
        error_msg += " and count ";
        error_msg += std::to_string(count);
        logger.LogError(error_msg);
        exit(-1);
    }
    physical_devices.resize(count);
    vk_result = vkEnumeratePhysicalDevices(m_vk_inst, &count, physical_devices.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                                "vkEnumeratePhysicalDevices with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        exit(-1);
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
        exit(-1);
    }

    vk_result = vkEnumerateDeviceExtensionProperties(m_vk_phys_dev, nullptr, &count, nullptr);
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                  "vkEnumerateDeviceExtensionProperties first time with error ";
        error_msg += vk_result;
        error_msg += " and count ";
        error_msg += std::to_string(count);
        logger.LogError(error_msg);
        exit(-1);
    }

    extension_properties.resize(count);
    vk_result = vkEnumerateDeviceExtensionProperties(m_vk_phys_dev, nullptr, &count, extension_properties.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                                "vkEnumerateDeviceExtensionProperties with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        exit(-1);
    }

    bool found_swapchain = false;
    enable_extension_count = 0;
    for (uint32_t ext = 0; ext < count; ext++) {
        if (!strcmp(extension_properties[ext].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
            found_swapchain = true;
            extensions_to_enable[enable_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }

    if (!found_swapchain) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to find necessary extension ";
        error_msg += VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        logger.LogError(error_msg);
        exit(-1);
    }

    vkGetPhysicalDeviceQueueFamilyProperties(m_vk_phys_dev, &queue_family_count, nullptr);
    if (queue_family_count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                  "vkGetPhysicalDeviceQueueFamilyProperties first time with count ";
        error_msg += std::to_string(queue_family_count);
        logger.LogError(error_msg);
        exit(-1);
    }

    std::vector<VkQueueFamilyProperties> queue_family_props;
    queue_family_props.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vk_phys_dev, &queue_family_count, queue_family_props.data());

    if (!window->CreateGfxWindow(m_vk_inst)) {
        exit(-1);
    }

    std::vector<VkBool32> present_support;
    present_support.resize(queue_family_count);
    for (uint32_t queue_family = 0; queue_family < queue_family_count; queue_family++) {
        m_vk_inst_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(m_vk_phys_dev, queue_family,
                                                                    window->GetSurface(),
                                                                    &present_support[queue_family]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    m_graphics_queue_family_index = UINT32_MAX;
    m_present_queue_family_index = UINT32_MAX;
    m_separate_present_queue = true;
    for (uint32_t queue_family = 0; queue_family < queue_family_count; queue_family++) {
        if ((queue_family_props[queue_family].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            m_graphics_queue_family_index = queue_family;
            if (present_support[queue_family] == VK_TRUE) {
                // We found one that supports both
                m_present_queue_family_index = queue_family;
                m_separate_present_queue = false;
                break;
            }
        } else if (VK_TRUE == present_support[queue_family]) {
            m_present_queue_family_index = queue_family;
        }
    }

    if (UINT32_MAX == m_graphics_queue_family_index || UINT32_MAX == m_present_queue_family_index) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to find either "
                  "a graphics or present queue for physical device.";
        logger.LogError(error_msg);
        exit(-1);
    }

    float queue_priorities[1] = { 0.0 };
    VkDeviceQueueCreateInfo queue_create_info[2] = {};
    queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].pNext = nullptr;
    queue_create_info[0].queueFamilyIndex = m_graphics_queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = queue_priorities;
    queue_create_info[0].flags = 0;
    if (m_separate_present_queue) {
        queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info[1].pNext = nullptr;
        queue_create_info[1].queueFamilyIndex = m_present_queue_family_index;
        queue_create_info[1].queueCount = 1;
        queue_create_info[1].pQueuePriorities = queue_priorities;
        queue_create_info[1].flags = 0;
    }

    VkDeviceCreateInfo device_create_info = { };
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.enabledExtensionCount = enable_extension_count;
    device_create_info.ppEnabledExtensionNames = (const char *const *)extensions_to_enable;
    device_create_info.pEnabledFeatures = nullptr;
    if (m_separate_present_queue) {
        device_create_info.queueCreateInfoCount = 2;
    }

    vk_result = vkCreateDevice(m_vk_phys_dev, &device_create_info, nullptr, &m_vk_device);
    if (vk_result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        logger.LogError("LgVulkanEngine::LgVulkanEngine failed vkCreateDevice could not find a "
                        "compatible Vulkan ICD");
        exit(-1);
    } else if (vk_result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        logger.LogError("LgVulkanEngine::LgVulkanEngine failed vkCreateDevice could not find "
                        "one or more extensions");
        exit(-1);
    } else if (vk_result) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed vkCreateDevice with ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        exit(-1);
    }

    layer_init_device_dispatch_table(m_vk_device, &m_vk_dev_dispatch_table, vkGetDeviceProcAddr);

    // Get the list of VkFormat's that are supported:
    count = 0;
    vk_result = m_vk_inst_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(m_vk_phys_dev,
                                                                            window->GetSurface(),
                                                                            &count, nullptr);
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                                "GetPhysicalDeviceSurfaceFormatsKHR with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        exit(-1);
    }

    std::vector<VkSurfaceFormatKHR> surface_formats;
    surface_formats.resize(count);
    vk_result = m_vk_inst_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(m_vk_phys_dev,
                                                                            window->GetSurface(),
                                                                            &count, surface_formats.data());
    if (VK_SUCCESS != vk_result || count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to query "
                                "GetPhysicalDeviceSurfaceFormatsKHR 2nd time with count ";
        error_msg += std::to_string(count);
        error_msg += " error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        exit(-1);
    }

    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
std::cout << "Forcing to BGRA8\n";
//        demo->format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
std::cout << "Found available formats\n";
//std::stringstream ss;
//for (uint32_t m = 0; m < count; m++) {
//        ss << "\t" << (uint32_t)(surface_formats[m]) << "\n";
//}
//std::cout << ss.str();
for (auto i = surface_formats.begin(); i != surface_formats.end(); ++i) {
uint32_t m =i->format;
//        ss << "\t" << *i << "\n";
std::cout << "\t" << m << std::endl;
}
//        demo->format = surfFormats[0].format;
    }
#if 0
    demo->color_space = surfFormats[0].colorSpace;

    m_quit = false;

    // Create semaphores to synchronize acquiring presentable buffers before
    // rendering and waiting for drawing to be complete before presenting
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
    };

    // Create fences that we can use to throttle if we get too far
    // ahead of the image presents
    VkFenceCreateInfo fence_ci = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (uint32_t i = 0; i < FRAME_LAG; i++) {
        err = vkCreateFence(m_vk_device, &fence_ci, NULL, &demo->fences[i]);
        assert(!err);

        err = vkCreateSemaphore(m_vk_device, &semaphoreCreateInfo, NULL,
                                &demo->image_acquired_semaphores[i]);
        assert(!err);

        err = vkCreateSemaphore(m_vk_device, &semaphoreCreateInfo, NULL,
                                &demo->draw_complete_semaphores[i]);
        assert(!err);

        if (demo->separate_present_queue) {
            err = vkCreateSemaphore(m_vk_device, &semaphoreCreateInfo, NULL,
                                    &demo->image_ownership_semaphores[i]);
            assert(!err);
        }
    }
    demo->frame_index = 0;

    // Get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(m_vk_phys_dev, &demo->memory_properties);
#endif
}

LgVulkanEngine::~LgVulkanEngine() {
    LgLogger &logger = LgLogger::getInstance();
    LgLogLevel level = logger.GetLogLevel();
    if (level > LG_LOG_DISABLE) {
        m_vk_inst_dispatch_table.DestroyDebugReportCallbackEXT(m_vk_inst, m_dbg_report_callback, nullptr);
    }

    vkDestroyDevice(m_vk_device, NULL);

    vkDestroySurfaceKHR(m_vk_inst, m_window->GetSurface(), NULL);

    vkDestroyInstance(m_vk_inst, NULL);
}

bool LgVulkanEngine::QueryWindowSystem(std::vector<VkExtensionProperties> &ext_props,
                                       uint32_t &ext_count, const char** desired_extensions) {
    bool khr_surface_found = false;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    bool khr_win32_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    bool khr_xlib_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    bool khr_xcb_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    bool khr_wayland_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
    bool khr_mir_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    bool khr_display_found = false;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    bool khr_android_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
    bool khr_mvk_ios_surface_found = false;
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    bool khr_mvk_macos_surface_found = false;
#endif
    LgLogger &logger = LgLogger::getInstance();

    if (ext_count == 0) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine incoming `ext_count` needs to contain"
                  " size of 'desired_extensions' array";
        logger.LogError(error_msg);
        return false;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < ext_props.size(); i++) {
        if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_surface_found = true;
            count++;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        } else if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_win32_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        } else if (!strcmp(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_xlib_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        } else if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_xcb_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        } else if (!strcmp(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_wayland_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
        } else if (!strcmp(VK_KHR_MIR_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_mir_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        } else if (!strcmp(VK_KHR_DISPLAY_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_display_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        } else if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_android_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
        } else if (!strcmp(VK_MVK_IOS_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_mvk_ios_surface_found = true;
            count++;
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
        } else if (!strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, ext_props[i].extensionName)) {
            khr_mvk_macos_surface_found = true;
            count++;
#endif
        }
    }

    if (count < 2) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine failed to find a platform extension (count = ";
        error_msg += count;
        error_msg += ").";
        logger.LogError(error_msg);
        return false;
    } else if (count > ext_count) {
        std::string error_msg = "LgVulkanEngine::LgVulkanEngine found too many extensions.  Expected < ";
        error_msg += ext_count;
        error_msg += ", but got ";
        error_msg += count;
        logger.LogError(error_msg);
        return false;
    } else {
        ext_count = 0;

        if (khr_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        if (khr_win32_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        if (khr_xlib_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        if (khr_xcb_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        if (khr_wayland_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
        if (khr_mir_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_MIR_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        if (khr_display_found) {
            desired_extensions[ext_count++] = VK_KHR_DISPLAY_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        if (khr_android_surface_found) {
            desired_extensions[ext_count++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
        if (khr_mvk_ios_surface_found) {
            desired_extensions[ext_count++] = VK_MVK_IOS_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
        if (khr_mvk_macos_surface_found) {
            desired_extensions[ext_count++] = VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
        }
#endif
    }

    return true;
}

int LgVulkanEngine::CompareGpus(VkPhysicalDeviceProperties &gpu_0,
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