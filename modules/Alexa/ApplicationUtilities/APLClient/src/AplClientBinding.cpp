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

#include "APLClient/AplCoreEngineLogBridge.h"
#include "APLClient/AplClientBinding.h"

namespace APLClient {

AplClientBinding::AplClientBinding(AplOptionsInterfacePtr options) : m_aplOptions{options} {
    m_aplConnectionManager = std::make_shared<AplCoreConnectionManager>(options);
    m_aplGuiRenderer.reset(new AplCoreGuiRenderer(options, m_aplConnectionManager));
    apl::LoggerFactory::instance().initialize(std::make_shared<AplCoreEngineLogBridge>(options));
}

bool AplClientBinding::shouldHandleMessage(const std::string& message) {
    return m_aplConnectionManager->shouldHandleMessage(message);
}

void AplClientBinding::handleMessage(const std::string& message) {
    m_aplConnectionManager->handleMessage(message);
}

void AplClientBinding::renderDocument(
    const std::string& document,
    const std::string& data,
    const std::string& viewports,
    const std::string& token) {
    m_aplGuiRenderer->renderDocument(document, data, viewports, token);
}

void AplClientBinding::clearDocument() {
    m_aplGuiRenderer->clearDocument();
}

void AplClientBinding::executeCommands(const std::string& jsonPayload, const std::string& token) {
    m_aplConnectionManager->executeCommands(jsonPayload, token);
}

void AplClientBinding::interruptCommandSequence() {
    m_aplGuiRenderer->interruptCommandSequence();
}

void AplClientBinding::requestVisualContext(unsigned int stateRequestToken) {
    m_aplConnectionManager->provideState(stateRequestToken);
}

void AplClientBinding::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    m_aplConnectionManager->dataSourceUpdate(sourceType, jsonPayload, token);
}

void AplClientBinding::onUpdateTick() {
    m_aplConnectionManager->onUpdateTick();
}
}  // namespace APLClient