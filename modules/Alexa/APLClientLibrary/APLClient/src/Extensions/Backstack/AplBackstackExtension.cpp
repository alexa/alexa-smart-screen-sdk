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
#include "APLClient/Extensions/Backstack/AplBackstackExtension.h"

namespace APLClient {
namespace Extensions {
namespace Backstack {

/// String to identify log entries originating from this file.
static const std::string TAG("AplBackstackExtension");

static const std::string ENVIRONMENT_RESPONSIBLE_FOR_BACK_BUTTON = "responsibleForBackButton";
static const std::string ENVIRONMENT_BACKSTACK = "backstack";
static const std::string SETTING_PROPERTY_BACKSTACK_ID = "backstackId";
static const std::string SETTING_PROPERTY_BACKSTACK_ARRAY_NAME = "backstackArrayName";
static const std::string COMMAND_GO_BACK_NAME = "GoBack";
static const std::string COMMAND_CLEAR_NAME = "Clear";
static const std::string PROPERTY_BACK_TYPE = "backType";
static const std::string PROPERTY_BACK_VALUE = "backValue";
static const std::string PROPERTY_BACK_TYPE_COUNT = "count";

AplBackstackExtension::AplBackstackExtension(std::shared_ptr<AplBackstackExtensionObserverInterface> observer) :
        m_observer{observer} {
    m_backstackArrayName = "";
    m_responsibleForBackButton = false;
}

void AplBackstackExtension::setResponsibleForBackButton(bool isResponsibleForBackButton) {
    m_responsibleForBackButton = isResponsibleForBackButton;
}

bool AplBackstackExtension::shouldCacheActiveDocument() {
    return !m_activeDocumentId.empty();
}

void AplBackstackExtension::addDocumentStateToBackstack(const AplDocumentStatePtr& documentState) {
    documentState->id = m_activeDocumentId;
    m_backstack.addDocumentState(documentState);
    clearActiveDocumentId();
}

void AplBackstackExtension::clearActiveDocumentId() {
    m_activeDocumentId = "";
}

void AplBackstackExtension::reset() {
    clearActiveDocumentId();
    m_backstack.clear();
}

bool AplBackstackExtension::handleBack() {
    if (!m_responsibleForBackButton) {
        return goBackCount(1);
    }
    return false;
}

std::string AplBackstackExtension::getUri() {
    return URI;
}

apl::Object AplBackstackExtension::getEnvironment() {
    auto env = std::make_shared<apl::ObjectMap>();
    env->emplace(ENVIRONMENT_RESPONSIBLE_FOR_BACK_BUTTON, m_responsibleForBackButton);
    env->emplace(ENVIRONMENT_BACKSTACK, m_backstack.getBackstackIdsArray());
    return apl::Object(env);
}

std::list<apl::ExtensionCommandDefinition> AplBackstackExtension::getCommandDefinitions() {
    std::list<apl::ExtensionCommandDefinition> extCmdDefs(
        {apl::ExtensionCommandDefinition(URI, COMMAND_GO_BACK_NAME)
             .allowFastMode(true)
             .property(PROPERTY_BACK_TYPE, PROPERTY_BACK_TYPE_COUNT, false)
             .property(PROPERTY_BACK_VALUE, 1, false),
         apl::ExtensionCommandDefinition(URI, COMMAND_CLEAR_NAME).allowFastMode(true)});
    return extCmdDefs;
}

std::list<apl::ExtensionEventHandler> AplBackstackExtension::getEventHandlers() {
    std::list<apl::ExtensionEventHandler> extensionEventHandlers({});
    return extensionEventHandlers;
}

std::unordered_map<std::string, apl::LiveObjectPtr> AplBackstackExtension::getLiveDataObjects() {
    auto liveObjects = std::unordered_map<std::string, apl::LiveObjectPtr>();
    if (!m_backstackArrayName.empty()) {
        liveObjects.emplace(m_backstackArrayName, m_backstack.getBackstackIds());
    }
    return liveObjects;
}

void AplBackstackExtension::applySettings(const apl::Object& settings) {
    // Reset to defaults
    clearActiveDocumentId();
    m_backstackArrayName = "";
    logMessage(LOGLEVEL_DEBUG, TAG, "backstack_settings", settings.toDebugString());
    /// Apply @c apl::Content defined settings
    if (settings.isMap()) {
        if (settings.has(SETTING_PROPERTY_BACKSTACK_ID)) {
            m_activeDocumentId = settings.get(SETTING_PROPERTY_BACKSTACK_ID).getString();
        }
        if (settings.has(SETTING_PROPERTY_BACKSTACK_ARRAY_NAME)) {
            m_backstackArrayName = settings.get(SETTING_PROPERTY_BACKSTACK_ARRAY_NAME).getString();
        }
    }
}

bool AplBackstackExtension::restoreDocumentState(const AplDocumentStatePtr& documentState) {
    if (documentState) {
        clearActiveDocumentId();
        m_observer->onRestoreDocumentState(documentState);
        return true;
    }
    return false;
}

bool AplBackstackExtension::goBackId(const std::string& id) {
    return restoreDocumentState(m_backstack.popDocuments(id));
}

bool AplBackstackExtension::goBackIndex(unsigned int index) {
    return restoreDocumentState(m_backstack.popDocumentsAtIndex(index));
}

bool AplBackstackExtension::goBackCount(unsigned int count) {
    return restoreDocumentState(m_backstack.popDocuments(count));
}

bool AplBackstackExtension::handleGoBack(const apl::Object& params) {
    if (confirmEventParams(TAG, {PROPERTY_BACK_TYPE, PROPERTY_BACK_VALUE}, params)) {
        auto backType = backTypeFromString(params.get(PROPERTY_BACK_TYPE).getString());
        auto backValue = params.get(PROPERTY_BACK_VALUE);
        switch (backType) {
            case AplBackType::COUNT:
                if (backValue.isNumber()) {
                    return goBackCount(backValue.getUnsigned());
                }
            case AplBackType::INDEX:
                if (backValue.isNumber()) {
                    return goBackIndex(backValue.getUnsigned());
                }
            case AplBackType::ID:
                if (backValue.isString()) {
                    return goBackId(backValue.getString());
                }
        }
    }
    return false;
}

void AplBackstackExtension::onExtensionEvent(
    const std::string& uri,
    const std::string& name,
    const apl::Object& source,
    const apl::Object& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    auto eventDebugString = getEventDebugString(uri, name, params);
    logMessage(LOGLEVEL_DEBUG, TAG, __func__, eventDebugString);

    bool succeeded = true;

    if (m_observer) {
        if (COMMAND_GO_BACK_NAME == name) {
            succeeded = handleGoBack(params);
        } else if (COMMAND_CLEAR_NAME == name) {
            m_backstack.clear();
        } else {
            logMessage(apl::LogLevel::kError, TAG, __func__, "Invalid Command: " + eventDebugString);
            succeeded = false;
        }
    } else {
        logMessage(apl::LogLevel::kError, TAG, __func__, "No Event Observer: " + eventDebugString);
        succeeded = false;
    }

    if (resultCallback) {
        resultCallback->onExtensionEventResult(event, succeeded);
    }
}

}  // namespace Backstack
}  // namespace Extensions
}  // namespace APLClient
