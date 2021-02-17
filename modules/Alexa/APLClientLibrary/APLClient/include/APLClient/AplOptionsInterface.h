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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLOPTIONSINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLOPTIONSINTERFACE_H_

#include <memory>
#include <chrono>
#include <string>

#include "AplRenderingEvent.h"
#include "Extensions/AplCoreExtensionEventCallbackResultInterface.h"

namespace APLClient {
using namespace APLClient::Extensions;
/// Enumeration of log levels sent by the APL client binding (DBG used to avoid conflicts with compiler defined macros)
enum class LogLevel { CRITICAL, ERROR, WARN, INFO, DBG, TRACE };

/**
 * The @c AplOptionsInterface defines the set of callbacks which users of the APL client library must provide, it will
 * be used to inform the consumer of certain state changes as well as requests for data or to pass messages to the
 * APL Viewhost.
 * @note This class is named "Options" to match the implementation of other APLCore integration methods, it does not
 * imply that this is only used for settings, or that it is an optional interface.
 */
class AplOptionsInterface {
public:
    /**
     * Virtual destructor
     */
    virtual ~AplOptionsInterface() = default;

    /**
     * Send the given payload to the APL Viewhost
     * @param token The APL token
     * @param payload
     */
    virtual void sendMessage(const std::string& token, const std::string& payload) = 0;

    /**
     * Requests that the APL viewhost is reset to render a new APL document
     * @param token The APL token
     */
    virtual void resetViewhost(const std::string& token) = 0;

    /**
     * Download the given resource
     * @param source The URI for the resource
     * @return The content of the resource
     */
    virtual std::string downloadResource(const std::string& source) = 0;

    /**
     * Retrieve the current timezone offset
     * @return The offset in milliseconds
     */
    virtual std::chrono::milliseconds getTimezoneOffset() = 0;

    /**
     * The given activity has started
     * @param token The APL token
     * @param source The activity type
     */
    virtual void onActivityStarted(const std::string& token, const std::string& source) = 0;

    /**
     * The given activity has ended
     * @param token The APL token
     * @param source The activity type
     */
    virtual void onActivityEnded(const std::string& token, const std::string& source) = 0;

    /**
     * An APL send event command was executed
     * @param token The APL token
     * @param event The event
     */
    virtual void onSendEvent(const std::string& token, const std::string& event) = 0;

    /**
     * Command execution has completed
     * @param token The APL token
     * @param result Whether the command executed to completion successfully
     */
    virtual void onCommandExecutionComplete(const std::string& token, bool result) = 0;

    /**
     * Rendering the APL document has completed
     * @param token The APL token
     * @param result The result of the rendering
     * @param error The error message if a failure occurred
     */
    virtual void onRenderDocumentComplete(const std::string& token, bool result, const std::string& error) = 0;

    /**
     * Called as a response to a @c requestVisualContext request
     * @param token The APL token
     * @param stateRequestToken The token which was passed during the call to @c requestVisualContext
     * @param context The visual context
     */
    virtual void onVisualContextAvailable(
        const std::string& token,
        unsigned int stateRequestToken,
        const std::string& context) = 0;

    /**
     * Called when the document idle timeout is set
     * @param timeout The timeout value
     */
    virtual void onSetDocumentIdleTimeout(const std::string& token, const std::chrono::milliseconds& timeout) = 0;

    /**
     * Called when an event occurs during APL rendering, generally used for metrics
     * @param token The APL token
     * @param event The event
     */
    virtual void onRenderingEvent(const std::string& token, AplRenderingEvent event) = 0;

    /**
     * A finish event occurred, the APL document should be removed
     * @param token The APL token
     */
    virtual void onFinish(const std::string& token) = 0;

    /**
     * A data source fetch request for lazy loading
     * @param token The APL token
     * @param type The type of the fetch request
     * @param payload The payload for the fetch request
     */
    virtual void onDataSourceFetchRequestEvent(
        const std::string& token,
        const std::string& type,
        const std::string& payload) = 0;

    /**
     * Extension Event Callback function
     * @param uri Extension uri
     * @param name Extension event name
     * @param source Map of the source object that raised the event
     * @param params Map of the user-specified properties
     * @param event Event number
     * @param resultCallback Pointer to result callback interface
     */
    virtual void onExtensionEvent(
        const std::string& aplToken,
        const std::string& uri,
        const std::string& name,
        const std::string& source,
        const std::string& params,
        unsigned int event,
        std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) = 0;

    /**
     * Handles a RuntimeError event
     * @param token The APL token
     * @param payload The payload for the error event
     */
    virtual void onRuntimeErrorEvent(const std::string& token, const std::string& payload) = 0;

    /**
     * Called when a message should be logged
     * @param level The log level
     * @param source The source of the message
     * @param message
     */
    virtual void logMessage(LogLevel level, const std::string& source, const std::string& message) = 0;

    /**
     * Returns the maximum number of concurrent downloads from the configs.
     */
    virtual int getMaxNumberOfConcurrentDownloads() = 0;
};

/// Convenience typedef
using AplOptionsInterfacePtr = std::shared_ptr<AplOptionsInterface>;

}  // namespace APLClient
#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLOPTIONSINTERFACE_H_
