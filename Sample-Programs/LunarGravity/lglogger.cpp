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

void LgLogger::LogInfo(std::string message) {
    std::string prefix = "LunarGravity INFO: ";

    if (m_log_level >= LG_LOG_INFO_WARN_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
    }
}

void LgLogger::LogWarning(std::string message) {
    std::string prefix = "LunarGravity WARNING: ";

    if (m_log_level >= LG_LOG_WARN_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
    }
}

void LgLogger::LogError(std::string message) {
    std::string prefix = "LunarGravity ERROR: ";

    if (m_log_level >= LG_LOG_ERROR) {
        if (m_output_cmdline) {
            std::cout << prefix << message << std::endl;
        }
        if (m_output_file) {
            m_file_stream << prefix << message << std::endl;
        }
    }
}
