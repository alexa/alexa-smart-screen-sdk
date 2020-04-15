/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <unordered_map>

#include <AVSCommon/Utils/JSON/JSONUtils.h>

#include "SampleApp/AplCoreGuiContentDownloadManager.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

static const std::string TAG{"AplCoreGuiContentDownloadManager"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

using namespace alexaClientSDK::avsCommon::avs::attachment;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::libcurlUtils;
using namespace alexaClientSDK::avsCommon::utils::sds;

/// Process attachment ID
static const std::string PROCESS_ATTACHMENT_ID = "import_download:";
/// A wait period for a polling loop that constantly check if a content fetcher finished fetching the payload or failed.
static const std::chrono::milliseconds WAIT_FOR_ACTIVITY_TIMEOUT{100};
/// Timeout to wait for a package to arrive from the content fetcher
static const std::chrono::minutes FETCH_TIMEOUT{5};
/// The number of bytes read from the attachment with each read in the read loop.
static const size_t CHUNK_SIZE(1024);

AplCoreGuiContentDownloadManager::AplCoreGuiContentDownloadManager(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface>
        httpContentFetcherInterfaceFactoryInterface,
    unsigned long cachePeriodInSeconds,
    unsigned long maxCacheSize) :
        m_contentFetcherFactory{httpContentFetcherInterfaceFactoryInterface},
        m_cachePeriod{std::chrono::seconds(cachePeriodInSeconds)},
        m_maxCacheSize{maxCacheSize} {
}

AplCoreGuiContentDownloadManager::CachedPackage::CachedPackage(
    std::chrono::system_clock::time_point importTime,
    std::string packageContent) {
    this->importTime = importTime;
    this->packageContent = packageContent;
}

std::string AplCoreGuiContentDownloadManager::retrievePackage(const std::string& source) {
    if (cachedPackagesMap.find(source) != cachedPackagesMap.end()) {
        if ((std::chrono::system_clock::now() - cachedPackagesMap[source].importTime) < m_cachePeriod) {
            ACSDK_DEBUG9(LX("retrievePackage").d("packageSource", "returnedFromCache"));
            return cachedPackagesMap[source].packageContent;
        }
    }
    std::string packageContent = downloadPackage(source);
    ACSDK_DEBUG9(LX("retrievePackage").d("packageSource", "downloadedFromSource"));

    cachedPackagesMap[source] = CachedPackage(std::chrono::system_clock::now(), packageContent);
    cleanUpCache();

    return packageContent;
}

void AplCoreGuiContentDownloadManager::cleanUpCache() {
    std::chrono::system_clock::time_point oldestTime = std::chrono::system_clock::time_point::max();
    std::string oldestSource;

    for (auto it = cachedPackagesMap.begin(); it != cachedPackagesMap.end();) {
        if ((std::chrono::system_clock::now() - it->second.importTime) > m_cachePeriod) {
            it = cachedPackagesMap.erase(it);
            ACSDK_DEBUG9(LX("cleanUpCache").d("deletedCacheEntry", "entryExpired"));

        } else {
            if (it->second.importTime < oldestTime) {
                oldestTime = it->second.importTime;
                oldestSource = it->first;
            }
            it++;
        }
    }

    if (cachedPackagesMap.size() > m_maxCacheSize) {
        cachedPackagesMap.erase(oldestSource);
        ACSDK_DEBUG9(LX("cleanUpCache").d("deletedCacheEntry", "maxCacheSizeReached"));
    }
}

std::string AplCoreGuiContentDownloadManager::downloadPackage(const std::string& source) {
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

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
