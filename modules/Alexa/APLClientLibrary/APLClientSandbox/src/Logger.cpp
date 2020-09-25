/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "../include/APLClientSandbox/Logger.h"

std::vector<std::shared_ptr<ILogWriter>> Logger::m_logSinks;

bool Logger::m_debug = false;

std::mutex Logger::m_mutex;

void Logger::setDebugLogging(bool shouldEnable) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_debug = shouldEnable;
}

void Logger::log(const std::string& message) {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto& sink : m_logSinks) {
        sink->write(message);
    }
}

void Logger::addSink(std::shared_ptr<ILogWriter> sink) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_logSinks.push_back(sink);
}

void Logger::removeSink(std::shared_ptr<ILogWriter> sink) {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_logSinks.begin(); it != m_logSinks.end(); ++it) {
        if (*it == sink) {
            m_logSinks.erase(it);
            return;
        }
    }
}