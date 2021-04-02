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

#include <acsdkCore/CoreComponent.h>
#include <acsdkManufactory/ComponentAccumulator.h>
#include <acsdkShared/SharedComponent.h>
#include <ContextManager/ContextManager.h>
#include <acsdkDefaultSampleApplicationOptions/DefaultSampleApplicationOptionsComponent.h>

#ifdef ACSDK_ACS_UTILS
#include <acsdkACSSampleApplicationOptions/ACSSampleApplicationOptionsComponent.h>
#else
#include <acsdkDefaultSampleApplicationOptions/DefaultSampleApplicationOptionsComponent.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>

#endif

#include "SampleApp/LocaleAssetsManager.h"
#include "SampleApp/SampleApplicationComponent.h"

#ifdef METRICS_EXTENSION
#include <MetricsExtension/MetricsExtension.h>
#else
#include <acsdkDefaultSampleApplicationOptions/NullMetricRecorder.h>
#endif

namespace alexaSmartScreenSDK {
namespace sampleApp {

using namespace alexaClientSDK;
using namespace alexaClientSDK::acsdkManufactory;
using namespace alexaClientSDK::avsCommon::avs::initialization;

Component<
    acsdkManufactory::Import<std::unique_ptr<avsCommon::utils::libcurlUtils::HttpPostInterface>>,
    acsdkManufactory::Import<std::shared_ptr<avsCommon::utils::DeviceInfo>>,
    acsdkManufactory::Import<std::shared_ptr<registrationManager::CustomerDataManager>>,
    std::shared_ptr<avsCommon::utils::logger::Logger>,
    std::shared_ptr<avsCommon::utils::metrics::MetricRecorderInterface>>
getSampleApplicationOptionsComponent() {
    return ComponentAccumulator<>()
        .addComponent(acsdkShared::getComponent())
#ifdef ANDROID_LOGGER
        .addPrimaryFactory(applicationUtilities::androidUtilities::AndroidLogger::getAndroidLogger)
#else
        .addPrimaryFactory(avsCommon::utils::logger::getConsoleLogger)
#endif
#ifdef METRICS_EXTENSION
        .addRetainedFactory(alexaSmartScreenSDK::metrics::MetricsExtension::createMetricRecorderInterface)
#else
        .addRetainedFactory(acsdkDefaultSampleApplicationOptions::NullMetricRecorder::createMetricRecorderInterface)
#endif
        ;
}

/**
 * Function that returns a factory to instantiate @c LocaleAssetsManagerInterface.
 *
 * @param requiresShutdownList - The vector of @c RequiresShutdown pointers to which the @c LocaleAssetsManager will be
 * added.
 * @return An std::function to instantiate @c LocaleAssetsManagerInterface.
 */
static std::function<std::shared_ptr<avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>(
    const std::shared_ptr<avsCommon::utils::configuration::ConfigurationNode>&)>
getCreateLocaleAssetsManagerInterface(
    std::vector<std::shared_ptr<avsCommon::utils::RequiresShutdown>>& requiresShutdownList) {
    return
        [&requiresShutdownList](const std::shared_ptr<avsCommon::utils::configuration::ConfigurationNode>& configNode) {
            auto manager = sampleApp::LocaleAssetsManager::createLocaleAssetsManager(configNode);
            if (manager) {
                requiresShutdownList.push_back(manager);
            }
            return manager;
        };
}

Component<
    std::shared_ptr<avsCommon::avs::initialization::AlexaClientSDKInit>,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface>,
    std::shared_ptr<avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>,
    std::shared_ptr<avsCommon::utils::configuration::ConfigurationNode>,
    std::shared_ptr<avsCommon::utils::DeviceInfo>,
    std::shared_ptr<registrationManager::CustomerDataManager>,
    std::shared_ptr<avsCommon::utils::metrics::MetricRecorderInterface>>
getComponent(
    std::unique_ptr<avsCommon::avs::initialization::InitializationParameters> initParams,
    std::vector<std::shared_ptr<avsCommon::utils::RequiresShutdown>>& requiresShutdownList) {
    return ComponentAccumulator<>()
#ifdef ACSDK_ACS_UTILS
        .addComponent(acsdkSampleApplication::getSampleApplicationOptionsComponent())
#else
        .addComponent(getSampleApplicationOptionsComponent())
#endif
        .addPrimaryFactory(AlexaClientSDKInit::getCreateAlexaClientSDKInit(std::move(initParams)))
        .addRetainedFactory(avsCommon::utils::configuration::ConfigurationNode::createRoot)
        .addUniqueFactory(avsCommon::utils::libcurlUtils::HttpPost::createHttpPostInterface)
        .addRetainedFactory(avsCommon::utils::timing::MultiTimer::createMultiTimer)
        .addRetainedFactory(getCreateLocaleAssetsManagerInterface(requiresShutdownList))
        .addRetainedFactory(contextManager::ContextManager::createContextManagerInterface)
        .addRetainedFactory(avsCommon::utils::DeviceInfo::createFromConfiguration)
        .addRetainedFactory(registrationManager::CustomerDataManager::createCustomerDataManager);
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
