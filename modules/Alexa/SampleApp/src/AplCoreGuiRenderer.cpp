/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AVSCommon/Utils/JSON/JSONUtils.h>

#include "SampleApp/AplCoreGuiRenderer.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

using namespace alexaClientSDK::avsCommon::avs::attachment;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::libcurlUtils;
using namespace alexaClientSDK::avsCommon::utils::sds;

using namespace smartScreenSDKInterfaces;

static const std::string TAG{"AplCoreGuiRenderer"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// CDN for alexa import packages (styles/resources/etc)
static const char* ALEXA_IMPORT_PATH = "https://d2na8397m465mh.cloudfront.net/packages/%s/%s/document.json";
/// The number of bytes read from the attachment with each read in the read loop.
static const size_t CHUNK_SIZE(1024);
/// Name of the mainTemplate parameter to which avs datasources binds to.
static const std::string DEFAULT_PARAM_BINDING = "payload";
/// Default string to attach to mainTemplate parameters.
static const std::string DEFAULT_PARAM_VALUE = "{}";

AplCoreGuiRenderer::AplCoreGuiRenderer(
    std::shared_ptr<AplCoreConnectionManager> aplCoreConnectionManager,
    std::shared_ptr<AplCoreGuiContentDownloadManager> aplCoreGuiContentDownloadManager) :
        m_isDocumentCleared{false},
        m_aplCoreConnectionManager{aplCoreConnectionManager},
        m_aplCoreGuiContentDownloadManager{aplCoreGuiContentDownloadManager} {
}

void AplCoreGuiRenderer::executeCommands(const std::string& jsonPayload, const std::string& token) {
    ACSDK_DEBUG5(LX("executeCommands").d("token", token).sensitive("payload", jsonPayload));
    m_aplCoreConnectionManager->executeCommands(jsonPayload, token);
}

void AplCoreGuiRenderer::interruptCommandSequence() {
    m_aplCoreConnectionManager->interruptCommandSequence();
}

void AplCoreGuiRenderer::renderDocument(
    const std::string& jsonPayload,
    const std::string& token,
    const std::string& windowId) {
    ACSDK_DEBUG5(LX("renderDocument").sensitive("payload", jsonPayload));

    m_isDocumentCleared = false;

    renderByAplCore(
        extractDocument(jsonPayload),
        extractData(jsonPayload),
        extractSupportedViewports(jsonPayload),
        token,
        windowId);
}

void AplCoreGuiRenderer::clearDocument() {
    m_isDocumentCleared = true;
}

std::string AplCoreGuiRenderer::extractDocument(const std::string& jsonPayload) {
    rapidjson::Document document;
    document.Parse(jsonPayload);

    std::string jsonDocument;
    if (!jsonUtils::retrieveValue(document, "document", &jsonDocument)) {
        ACSDK_ERROR(LX(__func__).m("Failed to extract document"));
        return "{}";
    }

    return jsonDocument;
}

std::string AplCoreGuiRenderer::extractData(const std::string& jsonPayload) {
    rapidjson::Document document;
    document.Parse(jsonPayload);

    std::string jsonData;
    if (!jsonUtils::retrieveValue(document, "datasources", &jsonData)) {
        ACSDK_WARN(LX(__func__).m("Failed to extract data"));
        jsonData = DEFAULT_PARAM_VALUE;
    }

    return jsonData;
}

std::string AplCoreGuiRenderer::extractSupportedViewports(const std::string& jsonPayload) {
    rapidjson::Document document;
    document.Parse(jsonPayload);

    std::string jsonData;
    rapidjson::Value::ConstMemberIterator jsonIt;
    if (!jsonUtils::findNode(document, "supportedViewports", &jsonIt)) {
        ACSDK_WARN(LX(__func__).m("Failed to retrieve supportedViewports data"));
        jsonData = DEFAULT_PARAM_VALUE;
    } else {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        jsonIt->value.Accept(writer);
        jsonData = sb.GetString();
    }

    return jsonData;
}

void AplCoreGuiRenderer::renderByAplCore(
    const std::string& document,
    const std::string& data,
    const std::string& supportedViewports,
    const std::string& token,
    const std::string& windowId) {
    auto startTime = std::chrono::system_clock::now();

    auto content = apl::Content::create(document);
    if (!content) {
        ACSDK_ERROR(LX("renderDocumentFailed").d("document", document).m("Unable to create content"));

        m_guiManager->handleRenderDocumentResult(token, false, "Unable to create content");
        return;
    }

    for (size_t i = 0; i < content->getParameterCount(); i++) {
        auto param = content->getParameterAt(i);
        if (param == DEFAULT_PARAM_BINDING) {
            content->addData(DEFAULT_PARAM_BINDING, data);
        } else {
            content->addData(param, DEFAULT_PARAM_VALUE);
        }
    }

    while (content->isWaiting() && !content->isError()) {
        auto packages = content->getRequestedPackages();
        for (auto& package : packages) {
            auto name = package.reference().name();
            auto version = package.reference().version();
            auto source = package.source();

            if (source.empty()) {
                char sourceBuffer[CHUNK_SIZE];
                snprintf(sourceBuffer, CHUNK_SIZE, ALEXA_IMPORT_PATH, name.c_str(), version.c_str());
                source = sourceBuffer;
            }

            auto packageContent = m_aplCoreGuiContentDownloadManager->retrievePackage(source);
            if (packageContent.empty()) {
                ACSDK_ERROR(LX("renderDocumentFailed")
                                .d("package", name)
                                .d("version", version)
                                .d("source", source)
                                .m("Import requested, could not be loaded"));

                m_guiManager->handleRenderDocumentResult(token, false, "Unresolved import");
                return;
            }
            ACSDK_DEBUG5(LX("renderDocument").d("package", name).m("Import requested, was loaded"));
            content->addPackage(package, packageContent);
        }
    }

    ACSDK_DEBUG9(
        LX("renderDocument")
            .d("downloadContentTimeInMs",
               std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime)
                   .count()));

    if (!content->isReady()) {
        ACSDK_ERROR(LX("renderDocumentFailed").m("Content is not ready"));

        m_guiManager->handleRenderDocumentResult(token, false, "Content is not ready");
        return;
    }

    if (!m_isDocumentCleared) {
        /**
         *  Only set the content if we haven't been cleared while building.
         */
        m_aplCoreConnectionManager->setSupportedViewports(supportedViewports);
        m_aplCoreConnectionManager->setContent(content, token, windowId);
    }
}

void AplCoreGuiRenderer::setGuiManager(const std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> guiManager) {
    m_guiManager = guiManager;
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
