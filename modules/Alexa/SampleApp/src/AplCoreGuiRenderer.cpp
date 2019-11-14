/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
/// Process attachment ID
static const std::string PROCESS_ATTACHMENT_ID = "import_download:";
/// A wait period for a polling loop that constantly check if a content fetcher finished fetching the payload or failed.
static const std::chrono::milliseconds WAIT_FOR_ACTIVITY_TIMEOUT{100};
/// Timeout to wait for a package to arrive from the content fetcher
static const std::chrono::minutes FETCH_TIMEOUT{5};
/// Name of the mainTemplate parameter to which avs datasources binds to.
static const std::string DEFAULT_PARAM_BINDING = "payload";
/// Default string to attach to mainTemplate parameters.
static const std::string DEFAULT_PARAM_VALUE = "{}";

AplCoreGuiRenderer::AplCoreGuiRenderer(
    std::shared_ptr<AplCoreConnectionManager> aplCoreConnectionManager,
    std::shared_ptr<HTTPContentFetcherFactory> httpContentFetcherFactory) :
        m_isDocumentCleared{false},
        m_aplCoreConnectionManager{aplCoreConnectionManager},
        m_contentFetcherFactory{httpContentFetcherFactory} {
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

std::string AplCoreGuiRenderer::downloadPackage(const std::string& source) {
    auto contentFetcher = m_contentFetcherFactory->create(source);
    contentFetcher->getContent(HTTPContentFetcherInterface::FetchOptions::ENTIRE_BODY);

    HTTPContentFetcherInterface::Header header = contentFetcher->getHeader(nullptr);
    if (!header.successful) {
        ACSDK_ERROR(LX(__func__).sensitive("source", source).m("getHeaderFailed"));
        return "";
    }

    if (!isStatusCodeSuccess(header.responseCode)) {
        ACSDK_ERROR(LX("downloadPackageFailed")
                        .d("statusCode", header.responseCode)
                        .d("reason", "nonSuccessStatusCodeFromGetHeader"));
        return "";
    }

    ACSDK_DEBUG9(LX("downloadPackage")
                     .d("contentType", header.contentType)
                     .d("statusCode", header.responseCode)
                     .sensitive("url", source)
                     .m("headersReceived"));

    auto stream = std::make_shared<InProcessAttachment>(PROCESS_ATTACHMENT_ID);
    std::shared_ptr<AttachmentWriter> streamWriter = stream->createWriter(WriterPolicy::BLOCKING);

    if (!contentFetcher->getBody(streamWriter)) {
        ACSDK_ERROR(LX("downloadPackageFailed").d("reason", "getBodyFailed"));
        return "";
    }

    auto startTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::steady_clock::now() - startTime;
    HTTPContentFetcherInterface::State contentFetcherState = contentFetcher->getState();
    while ((FETCH_TIMEOUT > elapsedTime) && (HTTPContentFetcherInterface::State::BODY_DONE != contentFetcherState) &&
           (HTTPContentFetcherInterface::State::ERROR != contentFetcherState)) {
        std::this_thread::sleep_for(WAIT_FOR_ACTIVITY_TIMEOUT);
        elapsedTime = std::chrono::steady_clock::now() - startTime;
        contentFetcherState = contentFetcher->getState();
    }

    if (FETCH_TIMEOUT <= elapsedTime) {
        ACSDK_ERROR(LX("downloadPackageFailed").d("reason", "waitTimeout"));
        return "";
    }

    if (HTTPContentFetcherInterface::State::ERROR == contentFetcherState) {
        ACSDK_ERROR(LX("downloadPackageFailed").d("reason", "receivingBodyFailed"));
        return "";
    }

    std::unique_ptr<AttachmentReader> reader = stream->createReader(ReaderPolicy::NONBLOCKING);

    std::string packageContent;
    auto readStatus = AttachmentReader::ReadStatus::OK;
    std::vector<char> buffer(CHUNK_SIZE, 0);
    bool streamClosed = false;
    AttachmentReader::ReadStatus previousStatus = AttachmentReader::ReadStatus::OK_TIMEDOUT;
    ssize_t bytesReadSoFar = 0;
    ssize_t bytesRead = -1;
    while (!streamClosed && bytesRead != 0) {
        bytesRead = reader->read(buffer.data(), buffer.size(), &readStatus);
        bytesReadSoFar += bytesRead;
        if (previousStatus != readStatus) {
            ACSDK_DEBUG9(LX(__func__).d("readStatus", readStatus));
            previousStatus = readStatus;
        }
        switch (readStatus) {
            case AttachmentReader::ReadStatus::CLOSED:
                streamClosed = true;
                if (bytesRead == 0) {
                    break;
                }
                /* FALL THROUGH - to add any data received even if closed */
            case AttachmentReader::ReadStatus::OK:
            case AttachmentReader::ReadStatus::OK_WOULDBLOCK:
            case AttachmentReader::ReadStatus::OK_TIMEDOUT:
                packageContent.append(buffer.data(), bytesRead);
                break;
            case AttachmentReader::ReadStatus::OK_OVERRUN_RESET:
                // Current AttachmentReader policy renders this outcome impossible.
                ACSDK_ERROR(LX("downloadPackageFailed").d("reason", "overrunReset"));
                break;
            case AttachmentReader::ReadStatus::ERROR_OVERRUN:
            case AttachmentReader::ReadStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE:
            case AttachmentReader::ReadStatus::ERROR_INTERNAL:
                ACSDK_ERROR(LX("downloadPackageFailed").d("reason", "readError"));
                return "";
        }
        if (0 == bytesRead) {
            ACSDK_DEBUG9(LX(__func__).m("alreadyReadAllBytes"));
        }
    }

    ACSDK_DEBUG9(LX("downloadPackage").d("URL", contentFetcher->getUrl()));

    return packageContent;
}

void AplCoreGuiRenderer::renderByAplCore(
    const std::string& document,
    const std::string& data,
    const std::string& supportedViewports,
    const std::string& token,
    const std::string& windowId) {
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

            auto packageContent = downloadPackage(source);
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
