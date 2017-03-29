/*
 * LunarGravity - lglogger.cpp
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

#include "lglogger.hpp"

VKAPI_ATTR VkBool32 VKAPI_CALL LoggerCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
                                              uint64_t srcObject, size_t location, int32_t msgCode,
                                              const char *pLayerPrefix, const char *pMsg, void *pUserData) {
    LgLogger &logger = LgLogger::getInstance();
    std::string message = "Layer: ";
    message += pLayerPrefix;
    message += ", Code: ";
    message += std::to_string(msgCode);
    message += ", Message: ";
    message += pMsg;

    if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        logger.LogWarning(message);
    } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        logger.LogPerf(message);
    } else if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        logger.LogError(message);
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        logger.LogDebug(message);
    } else {
        logger.LogInfo(message);
    }

    return false;
}

LgLogger::LgLogger() {
    m_output_cmdline = true;
    m_output_file = false;
    m_log_level = LG_LOG_WARN_ERROR;
}

LgLogger::~LgLogger() {
    if (m_output_file) {
        m_file_stream.close();
    }
}

void LgLogger::SetFileOutput(std::string output_file) {
    if (output_file.size() > 0) {
        m_file_stream.open(output_file);
        if (m_file_stream.fail()) {
            std::cerr << "Error failed opening output file stream for "
                      << output_file << std::endl;
            m_output_file = false;
        } else {
            m_output_file = true;
        }
    }
}

void LgLogger::LogDebug(std::string message) {
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_DEBUG, "LunarGravity", "%s", message.c_str());
#else
    std::string prefix = "LunarGravity DEBUG: ";
    if (m_log_level >= LG_LOG_ALL) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
    }
#endif
}

void LgLogger::LogInfo(std::string message) {
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "LunarGravity", "%s", message.c_str());
#else
    std::string prefix = "LunarGravity INFO: ";
    if (m_log_level >= LG_LOG_INFO_WARN_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
    }
#endif
}

void LgLogger::LogWarning(std::string message) {
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_WARN, "LunarGravity", "%s", message.c_str());
#else
    std::string prefix = "LunarGravity WARNING: ";
    if (m_log_level >= LG_LOG_WARN_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
#ifdef _WIN32
        if (m_enable_popups) {
            MessageBox(NULL, message.c_str(), prefix.c_str(), MB_OK);
        }
#endif
    }
#endif
}

void LgLogger::LogPerf(std::string message) {
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_WARN, "LunarGravity", "%s", message.c_str());
#else
    std::string prefix = "LunarGravity PERF: ";
    if (m_log_level >= LG_LOG_WARN_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
    }
#endif
}

void LgLogger::LogError(std::string message) {
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "LunarGravity", "%s", message.c_str());
#else
    std::string prefix = "LunarGravity ERROR: ";
    if (m_log_level >= LG_LOG_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
#ifdef _WIN32
        if (m_enable_popups) {
            MessageBox(NULL, message.c_str(), prefix.c_str(), MB_OK);
        }
#endif
    }
#endif
}
