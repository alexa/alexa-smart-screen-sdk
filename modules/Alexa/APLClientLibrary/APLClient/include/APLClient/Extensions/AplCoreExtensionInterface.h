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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONINTERFACE_H

#include <string>
#include <list>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#undef DEBUG
#undef TRUE
#undef FALSE
#include <apl/apl.h>
#include <apl/content/extensioncommanddefinition.h>
#include <apl/content/extensioneventhandler.h>
#pragma pop_macro("DEBUG")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma GCC diagnostic pop
#include "AplCoreExtensionEventCallbackInterface.h"
#include "AplCoreExtensionEventHandlerInterface.h"

namespace APLClient {
namespace Extensions {

/// Static cast of apl::LogLevel::DEBUG
static apl::LogLevel LOGLEVEL_DEBUG = static_cast<apl::LogLevel>(1);

/**
 * Log message through APL Core Logger
 * @param logLevel The @c apl::LogLevel.
 * @param file The source file of the message.
 * @param source The source name of the message.
 * @param message The message to log.
 */
static inline void logMessage(
    apl::LogLevel logLevel,
    const std::string& file,
    const std::string& source,
    const std::string& message) {
    apl::LoggerFactory::instance().getLogger(logLevel, file, source).log(message.c_str());
}

/**
 * Interface for an APL Extension that can be registered with AplCore and exposed to a runtime client.
 * Extensions are optional enhancements to an APL runtime that provide additional sources of data, commands,
 * and event handlers.
 *
 * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-extensions.html
 */
class AplCoreExtensionInterface : public virtual AplCoreExtensionEventCallbackInterface {
public:
    virtual ~AplCoreExtensionInterface() = default;

    /**
     * Gets the Uri of the Extension
     */
    virtual std::string getUri() = 0;

    /**
     * Gets the environment configuration of the Extension to be registered with @c apl::RootConfig
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/rootconfig.h
     */
    virtual apl::Object getEnvironment() = 0;

    /**
     * Get the list of Extension Command Definitions for the Extension
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/extensioncommanddefinition.h
     * @return list of Extension Command Definitions
     */
    virtual std::list<apl::ExtensionCommandDefinition> getCommandDefinitions() = 0;

    /**
     * Get the list of Event Handlers for the Extension
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/extensioneventhandler.h
     * @return list of Event Handlers of the Extension
     */
    virtual std::list<apl::ExtensionEventHandler> getEventHandlers() = 0;

    /**
     * Get the map of @c apl::LiveObjects that the extension provides to the APL data-binding context
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/src/livedata/livedataobject.cpp
     * @return map of LiveObject Pointers and their associated data-binding name.
     */
    virtual std::unordered_map<std::string, apl::LiveObjectPtr> getLiveDataObjects() = 0;

    /**
     * Apply extension settings retrieved from @c apl::Content
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/content.h#L161
     * @param settings the settings object
     */
    virtual void applySettings(const apl::Object& settings) = 0;

    /**
     * Set Event Handler for the extension.
     * This handler should handle invoking the Extension Event Handler on @c apl::RootContext
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/engine/rootcontext.h
     * @param eventHandler The @c AplCoreExtensionEventHandlerInterface
     */
    virtual void setEventHandler(std::shared_ptr<AplCoreExtensionEventHandlerInterface> eventHandler) {
        m_eventHandler = eventHandler;
    };

protected:
    /**
     * Internal utility function for generating event debug string.
     * @param uri Event uri.
     * @param name Event name.
     * @param params Event params.
     * @return Composite debug string of event values.
     */
    virtual std::string getEventDebugString(
        const std::string& uri,
        const std::string& name,
        const apl::Object& params) {
        return "< " + uri + "::" + name + "::" + params.toDebugString() + " >";
    };

    /**
     * Internal utility function for event param validation.
     * @param tag The file tag for the event params.
     * @param expectedParams The list if expected params.
     * @param params The received params.
     * @return True if the expected params are found.
     */
    virtual bool confirmEventParams(
        const std::string& tag,
        const std::list<std::string>& expectedParams,
        const apl::Object& params) {
        std::list<std::string> missingParams;
        if (params.isMap()) {
            for (auto& param : expectedParams) {
                if (!params.has(param)) {
                    missingParams.push_back(param);
                }
            }
        } else {
            missingParams = expectedParams;
        }

        if (!missingParams.empty()) {
            std::string missingParamsString = "Missing Params";
            for (auto& param : missingParams) {
                missingParamsString += (" : " + param);
            }
            logMessage(apl::LogLevel::kError, tag, __func__, missingParamsString);
            return false;
        }
        return true;
    };

    /// The pointer to the @c AplCoreExtensionEventHandlerInterface
    std::shared_ptr<AplCoreExtensionEventHandlerInterface> m_eventHandler;
};

}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONINTERFACE_H
