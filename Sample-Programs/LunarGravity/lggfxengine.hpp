/*
 * LunarGravity - lggfxengine.hpp
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

class LgWindow;
struct VulkanString;

class LgGraphicsEngine {
    public:

        // Create a protected constructor
        LgGraphicsEngine(const std::string &app_name, uint16_t app_version, bool validate, LgWindow &window);

        // We don't want any copy constructors
        LgGraphicsEngine(const LgGraphicsEngine &gfx_engine) = delete;
        LgGraphicsEngine &operator=(const LgGraphicsEngine &gfx_engine) = delete;

        // Make the destructor public
        virtual ~LgGraphicsEngine();

    protected:
 
    private:

        VkInstance m_vk_inst;
        bool m_validation_enabled;
        bool m_debug_enabled;
};
