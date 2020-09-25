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

#include <string>
#include "APLClient/Extensions/AplCoreExtensionManager.h"

namespace APLClient {
namespace Extensions {

/// String to identify log entries originating from this file.
static const std::string TAG("AplCoreExtensionManager");

std::shared_ptr<AplCoreExtensionInterface> AplCoreExtensionManager::getExtension(const std::string& uri) {
    logMessage(LOGLEVEL_DEBUG, TAG, __func__, uri);
    if (m_Extensions.find(uri) != m_Extensions.end()) {
        return m_Extensions[uri];
    }
    logMessage(LOGLEVEL_DEBUG, TAG, "No registered Extension", uri);
    return nullptr;
}

void AplCoreExtensionManager::addExtension(std::shared_ptr<AplCoreExtensionInterface> extension) {
    if (!getExtension(extension->getUri())) {
        m_Extensions.insert({extension->getUri(), extension});
    }
}

void AplCoreExtensionManager::registerRequestedExtension(const std::string& uri, apl::RootConfig& config) {
    if (auto extension = getExtension(uri)) {
        logMessage(LOGLEVEL_DEBUG, TAG, "registerRequestedExtension", uri);
        config.registerExtension(uri);
        config.registerExtensionEnvironment(extension->getUri(), extension->getEnvironment());
        for (auto& command : extension->getCommandDefinitions()) {
            logMessage(LOGLEVEL_DEBUG, TAG, "registerExtensionCommand", command.toDebugString());
            config.registerExtensionCommand(command);
        }
        for (auto& handler : extension->getEventHandlers()) {
            logMessage(LOGLEVEL_DEBUG, TAG, "registerExtensionEventHandler", handler.toDebugString());
            config.registerExtensionEventHandler(handler);
        }
        // Add Extension LiveData Objects to config
        auto extensionLiveObjects = extension->getLiveDataObjects();
        auto it = extensionLiveObjects.begin();
        while (it != extensionLiveObjects.end())
        {
            config.liveData(it->first, it->second);
            it++;
        }
    }
}

void AplCoreExtensionManager::onExtensionEvent(
    const std::string& uri,
    const std::string& name,
    const apl::Object& source,
    const apl::Object& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    logMessage(LOGLEVEL_DEBUG, TAG, "extensionEvent", +"< " + uri + "::" + name + "::" + params.toDebugString() + " >");
    if (auto extension = getExtension(uri)) {
        extension->onExtensionEvent(uri, name, source, params, event, resultCallback);
    } else if (resultCallback) {
        resultCallback->onExtensionEventResult(event, false);
    }
}

}  // namespace Extensions
}  // namespace APLClient
