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
#include <fstream>

#include <rapidjson/document.h>

#include "APLClient/AplCoreGuiRenderer.h"

namespace APLClient {

/// CDN for alexa import packages (styles/resources/etc)
/// (https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-document.html#import)
static const char* ALEXA_IMPORT_PATH = "https://d2na8397m465mh.cloudfront.net/packages/%s/%s/document.json";
/// The number of bytes read from the attachment with each read in the read loop.
static const size_t CHUNK_SIZE(1024);
/// Name of the mainTemplate parameter to which avs datasources binds to.
static const std::string DEFAULT_PARAM_BINDING = "payload";
/// Default string to attach to mainTemplate parameters.
static const std::string DEFAULT_PARAM_VALUE = "{}";

AplCoreGuiRenderer::AplCoreGuiRenderer(AplConfigurationPtr config, AplCoreConnectionManagerPtr aplCoreConnectionManager)
        : m_aplConfiguration{config},
          m_aplCoreConnectionManager{aplCoreConnectionManager},
          m_isDocumentCleared{false} {

}

void AplCoreGuiRenderer::executeCommands(const std::string& jsonPayload, const std::string& token) {
    m_aplCoreConnectionManager->executeCommands(jsonPayload, token);
}

void AplCoreGuiRenderer::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    m_aplCoreConnectionManager->dataSourceUpdate(sourceType, jsonPayload, token);
}

void AplCoreGuiRenderer::interruptCommandSequence() {
    m_aplCoreConnectionManager->interruptCommandSequence();
}

void AplCoreGuiRenderer::renderDocument(
    const std::string& document,
    const std::string& data,
    const std::string& supportedViewports,
    const std::string& token) {
    m_isDocumentCleared = false;

    auto metricsRecorder = m_aplConfiguration->getMetricsRecorder();
    auto aplOptions = m_aplConfiguration->getAplOptions();
    auto tContentCreate = metricsRecorder->createTimer(
            Telemetry::AplMetricsRecorderInterface::LATEST_DOCUMENT,
            Telemetry::AplRenderingSegment::kContentCreation);
    auto cImports = metricsRecorder->createCounter(
            Telemetry::AplMetricsRecorderInterface::LATEST_DOCUMENT,
            "APL-Web.Content.imports");
    auto cError = metricsRecorder->createCounter(
            Telemetry::AplMetricsRecorderInterface::LATEST_DOCUMENT,
            "APL-Web.Content.error");
    tContentCreate->start();
    auto content = apl::Content::create(std::move(document));
    if (!content) {
        aplOptions->logMessage(LogLevel::ERROR, "renderByAplCoreFailed", "Unable to create content");

        tContentCreate->fail();
        aplOptions->onRenderDocumentComplete(token, false, "Unable to create content");
        return;
    }

    std::map<std::string, apl::JsonData> params;
    apl::JsonData sourcesData(data);
    if (sourcesData.get().IsObject()) {
        for (auto objIt = sourcesData.get().MemberBegin(); objIt != sourcesData.get().MemberEnd(); objIt++) {
            params.emplace(objIt->name.GetString(), objIt->value);
        }
    }

    for (size_t idx = 0; idx < content->getParameterCount(); idx++) {
        auto parameterName = content->getParameterAt(idx);
        if (parameterName == DEFAULT_PARAM_BINDING) {
            content->addData(parameterName, data);
        } else if (params.find(parameterName) != params.end()) {
            content->addData(parameterName, params.at(parameterName).toString());
        } else {
            content->addData(parameterName, DEFAULT_PARAM_VALUE);
        }
    }

    std::unordered_map<uint32_t, std::future<std::string>> packageContentByRequestId;
    std::unordered_map<uint32_t, apl::ImportRequest> packageRequestByRequestId;
    while (content->isWaiting() && !content->isError()) {
        auto packages = content->getRequestedPackages();
        cImports->incrementBy(packages.size());
        unsigned int count = 0;
        for (auto& package : packages) {
            auto name = package.reference().name();
            auto version = package.reference().version();
            auto source = package.source();

            if (source.empty()) {
                char sourceBuffer[CHUNK_SIZE];
                snprintf(sourceBuffer, CHUNK_SIZE, ALEXA_IMPORT_PATH, name.c_str(), version.c_str());
                source = sourceBuffer;
            }

            auto packageContentPromise =
                async(std::launch::async, &AplOptionsInterface::downloadResource, aplOptions, source);
            packageContentByRequestId.insert(std::make_pair(package.getUniqueId(), std::move(packageContentPromise)));
            packageRequestByRequestId.insert(std::make_pair(package.getUniqueId(), package));
            count++;

            // if we reach the maximum number of concurrent downloads or already go through all packages, wait for them
            // to finish
            if (count % aplOptions->getMaxNumberOfConcurrentDownloads() == 0 || packages.size() == count) {
                for (auto& kvp : packageContentByRequestId) {
                    auto packageContent = kvp.second.get();
                    if (packageContent.empty()) {
                        aplOptions->logMessage(
                            LogLevel::ERROR, "renderByAplCoreFailed", "Could not be retrieve requested import");

                        aplOptions->onRenderDocumentComplete(token, false, "Unresolved import");
                        tContentCreate->fail();
                        return;
                    }
                    content->addPackage(packageRequestByRequestId.at(kvp.first), packageContent);
                }
                packageContentByRequestId.clear();
                packageRequestByRequestId.clear();
            }
        }
    }

    if (content->isError()) {
        cError->increment();
    }

    if (!content->isReady()) {
        aplOptions->logMessage(LogLevel::ERROR, "renderByAplCoreFailed", "Content is not ready");

        aplOptions->onRenderDocumentComplete(token, false, "Content is not ready");
        tContentCreate->fail();
        return;
    }

    tContentCreate->stop();
    metricsRecorder->flush();

    if (!m_isDocumentCleared) {
        /**
         *  Only set the content if we haven't been cleared while building.
         */
        m_aplCoreConnectionManager->setSupportedViewports(supportedViewports);
        m_aplCoreConnectionManager->setContent(content, token);
    }
}

void AplCoreGuiRenderer::clearDocument() {
    m_isDocumentCleared = true;
    m_aplCoreConnectionManager->reset();
}

}  // namespace APLClient
