/*
 * LunarGravity - lgvulkanengine.hpp
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

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

#include "lggfxengine.hpp"

struct VulkanString;

class LgVulkanEngine : public LgGraphicsEngine {
    public:

        // Create a protected constructor
        LgVulkanEngine(const std::string &app_name, uint16_t app_version, bool validate, LgWindow *window);

        // We don't want any copy constructors
        LgVulkanEngine(const LgVulkanEngine &gfx_engine) = delete;
        LgVulkanEngine &operator=(const LgVulkanEngine &gfx_engine) = delete;

        // Make the destructor public
        virtual ~LgVulkanEngine();

    protected:
 
    private:

        bool QueryWindowSystem(std::vector<VkExtensionProperties> &ext_props,
                               uint32_t &ext_count, const char** desired_extensions);
        int CompareGpus(VkPhysicalDeviceProperties &gpu_0, VkPhysicalDeviceProperties &gpu_1);

        // Vulkan Instance items
        VkInstance m_vk_inst;
        bool m_validation_enabled;
        bool m_debug_enabled;
        VkLayerInstanceDispatchTable m_vk_inst_dispatch_table;
        VkDebugReportCallbackEXT m_dbg_report_callback;

        // Vulkan Physical Device items
        VkPhysicalDevice m_vk_phys_dev;
        bool m_separate_present_queue;
        uint32_t m_graphics_queue_family_index;
        uint32_t m_present_queue_family_index;

        // Vulkan Logical Device items
        VkDevice m_vk_device;
        VkLayerDispatchTable m_vk_dev_dispatch_table;
};
