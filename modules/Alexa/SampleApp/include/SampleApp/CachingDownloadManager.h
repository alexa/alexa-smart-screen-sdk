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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_CACHINGDOWNLOADMANAGER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_CACHINGDOWNLOADMANAGER_H_

#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/SDKInterfaces/Storage/MiscStorageInterface.h>

#include <RegistrationManager/CustomerDataHandler.h>
#include <RegistrationManager/CustomerDataManager.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

class CachingDownloadManager : public alexaClientSDK::registrationManager::CustomerDataHandler {
public:
    /**
     * Constructor.
     *
     * @param httpContentFetcherFactory Pointer to a http content fetcher factory for making download requests
     * @param cachePeriodInSeconds Number of seconds to reuse cache for downloaded packages
     * @param maxCacheSize Maximum cache size for caching downloaded packages
     * @param miscStorage Wrapper to read and write to misc stor=age database
     */
    CachingDownloadManager(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface>
            httpContentFetcherInterfaceFactoryInterface,
        unsigned long cachePeriodInSeconds,
        unsigned long maxCacheSize,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> miscStorage,
        const std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager);

    /**
     * Method that should be called when requesting content
     *
     * @param source URL
     * @return content - either from cache or from source
     */
    std::string retrieveContent(const std::string& source);

    /**
     * Class to define a cached content item
     */
    class CachedContent {
    public:
        /**
         * Time when the content was put into cache
         */
        std::chrono::system_clock::time_point importTime;
        /**
         * Content of the item
         */
        std::string content;

        CachedContent() = default;

        /**
         * Constructor
         *
         * @param importTime Time when the item was inserted into cache
         * @param content The content of the item
         */
        CachedContent(std::chrono::system_clock::time_point importTime, std::string content);
    };

private:
    /**
     * Downloads content requested by import from provided URL from source.
     * @param source URL
     * @return content from source
     */
    std::string downloadFromSource(const std::string& source);
    /**
     * Scans the cache to remove all expired entries, and evict the oldest entry if cache is full
     */
    void cleanUpCache();
    /**
     * Write the downloaded content to storage.
     * @param source URL.
     * @param content CachedContent object.
     */
    void writeToStorage(std::string source, CachedContent content);

    /// @name CustomerDataHandler Function
    /// @{
    void clearData() override;
    /// @}

    /**
     * Remove the downloaded content from storage.
     * @param source  source URL.
     */
    void removeFromStorage(std::string source);
    /**
     * Used to create objects that can fetch remote HTTP content.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface>
        m_contentFetcherFactory;
    /**
     * Reuse time for caching of downloaded content
     */
    std::chrono::duration<double> m_cachePeriod;
    /**
     * Max numbers of entries in cache for downloaded content
     */
    unsigned long m_maxCacheSize;
    /**
     * The hashmap that maps the source url to a CachedContent
     */
    std::unordered_map<std::string, CachingDownloadManager::CachedContent> cachedContentMap;
    /**
     * The mutex for cachedContentMap
     */
    std::mutex cachedContentMapMutex;
    /**
     * The wrapper to read and write to local misc storage.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> m_miscStorage;
    /**
     * An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
     */
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

/**
 * This is a function to convert the @c CachedContent to a string.
 */
inline std::string cachedContentToString(CachingDownloadManager::CachedContent content, std::string delimiter) {
    auto time = std::chrono::duration_cast<std::chrono::seconds>(content.importTime.time_since_epoch()).count();
    return std::to_string(time) + delimiter + content.content;
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_CACHINGDOWNLOADMANAGER_H_