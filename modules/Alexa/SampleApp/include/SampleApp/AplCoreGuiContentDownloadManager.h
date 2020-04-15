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

#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

class AplCoreGuiContentDownloadManager {
public:
    /**
     * Constructor.
     *
     * @param httpContentFetcherFactory Pointer to a http content fetcher factory for making requests for APL imports
     * @param cachePeriodInSeconds Number of seconds to reuse cache for downloaded packages
     * @param maxCacheSize Maximum cache size for caching downloaded packages
     */
    AplCoreGuiContentDownloadManager(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface>
            httpContentFetcherInterfaceFactoryInterface,
        unsigned long cachePeriodInSeconds,
        unsigned long maxCacheSize);

    /**
     * Method that should be called when requesting packages
     *
     * @param source URL
     * @return package content - either from cache or from source
     */
    std::string retrievePackage(const std::string& source);

    /**
     * Class to define a cached package
     */
    class CachedPackage {
    public:
        /**
         * Time when the package was put into cache
         */
        std::chrono::system_clock::time_point importTime;
        /**
         * Content of the package
         */
        std::string packageContent;

        CachedPackage() = default;

        /**
         * Constructor
         *
         * @param importTime Time when the package was inserted into cache
         * @param packageContent The content of the package
         */
        CachedPackage(std::chrono::system_clock::time_point importTime, std::string packageContent);
    };

private:
    /**
     * Downloads package requested by import from provided URL from source.
     * @param source URL
     * @return package content from source
     */
    std::string downloadPackage(const std::string& source);
    /**
     * Scans the cache to remove all expired entries, and evict the oldest entry if cache is full
     */
    void cleanUpCache();
    /**
     * Used to create objects that can fetch remote HTTP content.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface>
        m_contentFetcherFactory;
    /**
     * Reuse time for caching of downloaded packages
     */
    std::chrono::duration<double> m_cachePeriod;
    /**
     * Max numbers of entries in cache for downloaded packages
     */
    unsigned long m_maxCacheSize;
    /**
     * The hashmap that maps the source url to a CachedPackage
     */
    std::unordered_map<std::string, AplCoreGuiContentDownloadManager::CachedPackage> cachedPackagesMap;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK