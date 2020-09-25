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

#ifndef APLCLIENTSANDBOX_INCLUDE_LOGGER_H_
#define APLCLIENTSANDBOX_INCLUDE_LOGGER_H_

#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

static const std::string LOG_DEBUG = "DEBUG";
static const std::string LOG_INFO = "INFO";
static const std::string LOG_WARN = "WARN";
static const std::string LOG_ERROR = "ERROR";

/// Interface which logging implementations should implement
class ILogWriter {
public:
    virtual ~ILogWriter() = default;

    virtual void write(const std::string& message) = 0;
};

/// A simple static logger
class Logger {
private:
    /**
     * Sends the given string to all log sinks
     * @param message
     */
    static void log(const std::string& message);

    /**
     * Formats the given log into a string
     * @tparam Ts type of parameters
     * @param level The log level
     * @param tag The tag for this log message
     * @param args The arguments
     * @return
     */
    template <typename... Ts>
    static std::string format(const std::string& level, const std::string& tag, Ts const&... args) {
        std::stringstream s;
        s << "[" << level << "] (" << tag << ")";
        int dummy[] = {0, ((s << " " << args), 0)...};
        static_cast<void>(dummy);  // Avoid warning for unused variable
        return s.str();
    }

    /// Vector of log sinks
    static std::vector<std::shared_ptr<ILogWriter>> m_logSinks;

    /// Indicates whether debug messages should be logged
    static bool m_debug;

    /// Mutex around log sinks
    static std::mutex m_mutex;

public:
    /**
     * Whether debug logs should be emitted
     * @param shouldEnable true - debug logging is enabled
     */
    static void setDebugLogging(bool shouldEnable);

    /**
     * Logs a debug message
     * @param tag The tag for the log message
     * @param args The message arguments
     */
    template <typename... Ts>
    static void debug(const std::string& tag, Ts const&... args) {
        if (m_debug) {
            log(format(LOG_DEBUG, tag, args...));
        }
    }

    /**
     * Logs an info message
     * @param tag The tag for the log message
     * @param args The message arguments
     */
    template <typename... Ts>
    static void info(const std::string& tag, Ts const&... args) {
        log(format(LOG_INFO, tag, args...));
    }

    /**
     * Logs a warning message
     * @param tag The tag for the log message
     * @param args The message arguments
     */
    template <typename... Ts>
    static void warn(const std::string& tag, Ts const&... args) {
        log(format(LOG_WARN, tag, args...));
    }

    /**
     * Logs an error message
     * @param tag The tag for the log message
     * @param args The message arguments
     */
    template <typename... Ts>
    static void error(const std::string& tag, Ts const&... args) {
        log(format(LOG_ERROR, tag, args...));
    }

    /**
     * Adds a logging sink
     * @param sink Pointer to the logging implementation
     */
    static void addSink(std::shared_ptr<ILogWriter> sink);

    /**
     * Removes a log sink
     * @param sink Pointer to the logging implementation
     */
    static void removeSink(std::shared_ptr<ILogWriter> sink);
};

#endif  // APLCLIENTSANDBOX_INCLUDE_LOGGER_H_
