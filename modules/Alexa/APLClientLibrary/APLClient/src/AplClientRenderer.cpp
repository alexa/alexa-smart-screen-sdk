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

#include <memory>

#include "APLClient/AplClientRenderer.h"
#include "APLClient/AplCoreEngineLogBridge.h"
#include "APLClient/Telemetry/AplMetricsRecorder.h"
#include "APLClient/Telemetry/NullAplMetricsRecorder.h"
#include "APLClient/Extensions/Backstack/AplBackstackExtension.h"

namespace {

static std::string TID_DELIMITER = "#TID#";
static std::string UNKNOWN_CLIENT_ID = "";
static std::string UNKNOWN_SKILL_ID = "";
static std::string METRIC_KIND_TIMER = "timer";
static std::string METRIC_KIND_COUNTER = "counter";

/*
 * The presentationToken format is as follows:
 * amzn{Amazon Common Id version}.{namespace}.{templateToken version}.{clientId}#TID#{SkillId}:{Skill-Sent-Token}:{Random-Number}.
 */

std::string extractClientId(const std::string& token) {
    if (token.empty()) {
        return UNKNOWN_CLIENT_ID;
    }

    auto delimiterPosition = token.find(TID_DELIMITER);
    if (delimiterPosition == std::string::npos) {
        return UNKNOWN_CLIENT_ID;
    }

    auto dotPosition = token.rfind(".", delimiterPosition);
    if (dotPosition == std::string::npos) {
        return UNKNOWN_CLIENT_ID;
    }

    auto clientIdPosition = dotPosition + 1;
    return token.substr(clientIdPosition, delimiterPosition - dotPosition);
}

std::string extractSkillId(const std::string& token) {
    if (token.empty()) {
        return UNKNOWN_CLIENT_ID;
    }

    auto delimiterPosition = token.find(TID_DELIMITER);
    if (delimiterPosition == std::string::npos) {
        return UNKNOWN_SKILL_ID;
    }

    auto skillIdPosition = delimiterPosition + TID_DELIMITER.length();
    auto colonPosition = token.find(":", skillIdPosition);
    if (colonPosition == std::string::npos) {
        return token.substr(skillIdPosition);
    } else {
        return token.substr(skillIdPosition, colonPosition - skillIdPosition);
    }
}

} // namespace

namespace APLClient {

using namespace APLClient::Telemetry;

AplClientRenderer::AplClientRenderer(AplConfigurationPtr config, std::string windowId)
        : m_aplConfiguration{config},
          m_windowId{windowId},
          m_aplConnectionManager{std::make_shared<AplCoreConnectionManager>(config)},
          m_aplGuiRenderer{new AplCoreGuiRenderer(config, m_aplConnectionManager)},
          m_lastReportedComplexity{0} {

}

bool AplClientRenderer::shouldHandleMessage(const std::string& message) {
    return m_aplConnectionManager->shouldHandleMessage(message);
}

void AplClientRenderer::handleMessage(const std::string& message) {
    m_aplConnectionManager->handleMessage(message);
}

void AplClientRenderer::renderDocument(
    const std::string& document,
    const std::string& data,
    const std::string& viewports,
    const std::string& token) {
    auto metricsRecorder = m_aplConfiguration->getMetricsRecorder();
    metricsRecorder->addMetadata(AplMetricsRecorderInterface::LATEST_DOCUMENT, "APL_TOKEN", token);

    std::string clientId = extractClientId(token);
    if (!clientId.empty()) {
        metricsRecorder->addMetadata(AplMetricsRecorderInterface::LATEST_DOCUMENT, "CLIENT_ID", clientId);
    }

    std::string skillId = extractSkillId(token);
    if (!skillId.empty()) {
        metricsRecorder->addMetadata(AplMetricsRecorderInterface::LATEST_DOCUMENT, "SKILL_ID", skillId);
    }

    m_aplToken = token;
    m_aplGuiRenderer->renderDocument(document, data, viewports, token);
}

void AplClientRenderer::clearDocument() {
    m_aplGuiRenderer->clearDocument();
}

void AplClientRenderer::executeCommands(const std::string& jsonPayload, const std::string& token) {
    m_aplConnectionManager->executeCommands(jsonPayload, token);
}

void AplClientRenderer::interruptCommandSequence() {
    m_aplGuiRenderer->interruptCommandSequence();
}

void AplClientRenderer::requestVisualContext(unsigned int stateRequestToken) {
    m_aplConnectionManager->provideState(stateRequestToken);
}

void AplClientRenderer::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    m_aplConnectionManager->dataSourceUpdate(sourceType, jsonPayload, token);
}

void AplClientRenderer::onUpdateTick() {
    m_aplConnectionManager->onUpdateTick();
}

const std::string AplClientRenderer::getWindowId() {
    return m_windowId;
}

const std::string AplClientRenderer::getCurrentAPLToken() {
    return m_aplToken;
}

void AplClientRenderer::addExtensions(std::unordered_set<std::shared_ptr<AplCoreExtensionInterface>> extensions) {
    m_aplConnectionManager->addExtensions(extensions);
}

void AplClientRenderer::onExtensionEvent(
    const std::string& uri,
    const std::string& name,
    const std::string& source,
    const std::string& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    m_aplConnectionManager->onExtensionEvent(uri, name, source, params, event, resultCallback);
}

AplDocumentStatePtr AplClientRenderer::getActiveDocumentState() {
    return m_aplConnectionManager->getActiveDocumentState();
}

void AplClientRenderer::restoreDocumentState(AplDocumentStatePtr documentState) {
    m_aplConnectionManager->restoreDocumentState(std::move(documentState));
}

void AplClientRenderer::onRenderDirectiveReceived(const std::chrono::steady_clock::time_point& receiveTime) {
    auto metricsRecorder = m_aplConfiguration->getMetricsRecorder();
    AplMetricsRecorderInterface::DocumentId document = metricsRecorder->registerDocument();
    m_renderTimer = metricsRecorder->createTimer(document, AplRenderingSegment::kRenderDocument);
    m_renderTimer->startedAt(receiveTime);

    auto counter = metricsRecorder->createCounter(document, "SmartScreenSDK.RenderDocumentReceived");
    counter->increment();
}

void AplClientRenderer::onRenderingEvent(AplRenderingEvent event) {
    const auto renderingStop = std::chrono::steady_clock::now();
    if (event == AplRenderingEvent::DOCUMENT_RENDERED) {
        m_aplConnectionManager->onDocumentRendered(renderingStop, m_lastReportedComplexity);
        if (m_renderTimer) {
            m_renderTimer->stoppedAt(renderingStop);
        }
        m_lastReportedComplexity = 0;
    } else if (event == AplRenderingEvent::RENDER_ABORTED) {
        if (m_renderTimer) {
            m_renderTimer->fail();
        }
        m_aplConfiguration->getMetricsRecorder()->flush();
    }
}

void AplClientRenderer::onMetricsReported(const std::string& jsonPayload) {
    rapidjson::Document doc;
    rapidjson::Value metricsPayload;

    auto aplOptions = m_aplConfiguration->getAplOptions();
    auto metricsRecorder = m_aplConfiguration->getMetricsRecorder();

    if (doc.Parse(jsonPayload.c_str()).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "onMetricsReportedFailed", "Error whilst parsing message");
        return;
    }

    if (!doc.HasMember("payload")) {
        aplOptions->logMessage(LogLevel::ERROR, "onMetricsReportedFailed", "Payload not found");
        return;
    }
    metricsPayload = doc["payload"];

    if (metricsPayload.GetType() != rapidjson::Type::kArrayType) {
        aplOptions->logMessage(LogLevel::ERROR, "onMetricsReportedFailed", "Payload is not an array");
        return;
    }

    for (rapidjson::SizeType i = 0; i < metricsPayload.Size(); i++) {
        const auto& jsonMetric = metricsPayload[i];

        if (!validateJsonMetric(jsonMetric)) {
            aplOptions->logMessage(LogLevel::ERROR, "onMetricsReportedFailed", "jsonMetric is invalid");
            return;
        }

        std::string kind = jsonMetric["kind"].GetString();
        std::string name = jsonMetric["name"].GetString();

        rapidjson::Value::ConstMemberIterator iterator;
        iterator = jsonMetric.FindMember("value");
        if (iterator == jsonMetric.MemberEnd()) {
            aplOptions->logMessage(LogLevel::ERROR, "onMetricsReportedFailed", "Unable to find jsonMetric value");
            return;
        }

        double d_value = 0;
        uint64_t i_value = 0;
        uint64_t value = 0;
        if (iterator->value.IsDouble()) {
            d_value = iterator->value.GetDouble();
            value = static_cast<uint64_t>(d_value);
        } else if (iterator->value.IsUint64()) {
            i_value = iterator->value.GetUint64();
            value = static_cast<uint64_t>(i_value);
        } else {
            aplOptions->logMessage(LogLevel::ERROR, "onMetricsReportedFailed", "jsonMetric contains incorrect type or value");
            return;
        }

        if (kind == METRIC_KIND_TIMER) {
            auto timer = metricsRecorder->createTimer(
                Telemetry::AplMetricsRecorderInterface::CURRENT_DOCUMENT,
                name);
            std::chrono::milliseconds elapsed(value);
            timer->elapsed(elapsed);
        } else {
            if (name == "componentComplexity") {
                m_lastReportedComplexity = value;
            } else {
                auto counter = metricsRecorder->createCounter(
                        Telemetry::AplMetricsRecorderInterface::CURRENT_DOCUMENT, name);
                counter->incrementBy(value);
            }
        }
    }

    metricsRecorder->flush();
}

bool AplClientRenderer::validateJsonMetric(
    const rapidjson::Value& jsonMetric) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!jsonMetric.HasMember("kind")) {
        aplOptions->logMessage(LogLevel::ERROR, "validateJsonMetricFailed", "missingMetricKind");
        return false;
    }
    std::string kind = jsonMetric["kind"].GetString();

    if (kind != METRIC_KIND_TIMER && kind != METRIC_KIND_COUNTER) {
        aplOptions->logMessage(LogLevel::ERROR, "validateJsonMetricFailed", "unsupportedMetricKind");
        return false;
    }

    if (!jsonMetric.HasMember("name")) {
        aplOptions->logMessage(LogLevel::ERROR, "validateJsonMetricFailed", "missingMetricName");
        return false;
    }

    if (!jsonMetric.HasMember("value")) {
        aplOptions->logMessage(LogLevel::ERROR, "validateJsonMetricFailed", "missingMetricValue");
        return false;
    }

    return true;
}

void AplClientRenderer::onTelemetrySinkUpdated(APLClient::Telemetry::AplMetricsSinkInterfacePtr sink) {
    Telemetry::AplMetricsRecorderInterfacePtr recorder;
    if (sink) {
        recorder = Telemetry::AplMetricsRecorder::create(sink);
    } else {
        recorder = std::make_shared<Telemetry::NullAplMetricsRecorder>();
    }
    m_aplConfiguration->setMetricsRecorder(recorder);
}

std::shared_ptr<APLClient::Extensions::AplCoreExtensionInterface> AplClientRenderer::getExtension(const std::string& uri) {
    return m_aplConnectionManager->getExtension(uri);
}


}  // namespace APLClient
