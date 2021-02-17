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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_SMARTSCREENCLIENT_INCLUDE_SMARTSCREENCLIENT_DEFAULTCLIENTCOMPONENT_H_
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_SMARTSCREENCLIENT_INCLUDE_SMARTSCREENCLIENT_DEFAULTCLIENTCOMPONENT_H_

#include <memory>

#include <ACL/Transport/MessageRouterFactoryInterface.h>
#include <acsdkAlerts/Storage/AlertStorageInterface.h>
#include <acsdkAlerts/AlertsCapabilityAgent.h>
#include <acsdkAudioPlayerInterfaces/AudioPlayerInterface.h>
#include <acsdkApplicationAudioPipelineFactoryInterfaces/ApplicationAudioPipelineFactoryInterface.h>
#include <acsdkDoNotDisturb/DoNotDisturbCapabilityAgent.h>
#include <acsdkExternalMediaPlayer/ExternalMediaPlayer.h>
#include <acsdkManufactory/Component.h>
#include <acsdkShutdownManagerInterfaces/ShutdownManagerInterface.h>
#include <acsdkStartupManagerInterfaces/StartupManagerInterface.h>
#include <AVSCommon/SDKInterfaces/AudioFocusAnnotation.h>
#include <Alexa/AlexaInterfaceMessageSender.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/AVSGatewayManagerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/ChannelVolumeFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExpectSpeechTimeoutHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/HTTPContentFetcherInterfaceFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/InternetConnectionMonitorInterface.h>
#include <AVSCommon/SDKInterfaces/Diagnostics/DiagnosticsInterface.h>
#include <AVSCommon/SDKInterfaces/LocaleAssetsManagerInterface.h>
#include <AVSCommon/SDKInterfaces/PowerResourceManagerInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderRegistrarInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointBuilderInterface.h>
#include <AVSCommon/SDKInterfaces/Storage/MiscStorageInterface.h>
#include <AVSCommon/SDKInterfaces/SystemTimeZoneInterface.h>
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/Timing/SystemClockMonitor.h>
#include <Captions/CaptionManagerInterface.h>
#include <CertifiedSender/CertifiedSender.h>
#include <CertifiedSender/MessageStorageInterface.h>
#include <InterruptModel/InterruptModel.h>
#include <RegistrationManager/CustomerDataManager.h>
#include <Settings/DeviceSettingsManager.h>
#include <Settings/Storage/DeviceSettingStorageInterface.h>

#include "SmartScreenClient/EqualizerRuntimeSetup.h"
#include "SmartScreenClient/StubApplicationAudioPipelineFactory.h"

namespace alexaSmartScreenSDK {
namespace smartScreenClient {

/**
 * Definition of a Manufactory component for the Smart Screen Client.
 */
using SmartScreenClientComponent = alexaClientSDK::acsdkManufactory::Component<
    std::shared_ptr<alexaClientSDK::acsdkAlerts::AlertsCapabilityAgent>,
    std::shared_ptr<alexaClientSDK::acsdkApplicationAudioPipelineFactoryInterfaces::ApplicationAudioPipelineFactoryInterface>,
    std::shared_ptr<alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerInterface>,
    std::shared_ptr<alexaClientSDK::acsdkEqualizerInterfaces::EqualizerRuntimeSetupInterface>,
    std::shared_ptr<alexaClientSDK::acsdkExternalMediaPlayer::ExternalMediaPlayer>,  /// Applications should not use this export.
    std::shared_ptr<alexaClientSDK::acsdkExternalMediaPlayerInterfaces::ExternalMediaPlayerInterface>,
    std::shared_ptr<alexaClientSDK::acsdkShutdownManagerInterfaces::ShutdownManagerInterface>,
    std::shared_ptr<alexaClientSDK::acsdkStartupManagerInterfaces::StartupManagerInterface>,
    std::shared_ptr<alexaClientSDK::afml::interruptModel::InterruptModel>,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeFactoryInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExpectSpeechTimeoutHandlerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>,
    alexaClientSDK::acsdkManufactory::Annotated<
        alexaClientSDK::avsCommon::sdkInterfaces::AudioFocusAnnotation,
        alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::InternetConnectionMonitorInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PowerResourceManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderRegistrarInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SystemTimeZoneInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AudioFactoryInterface>,
        alexaClientSDK::acsdkManufactory::Annotated<
            alexaClientSDK::avsCommon::sdkInterfaces::endpoints::DefaultEndpointAnnotation,
            alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointBuilderInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo>,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode>,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface>,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::timing::SystemClockMonitor>,
    std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender>,
    std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>,
    std::shared_ptr<alexaClientSDK::captions::CaptionManagerInterface>,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender>,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager>,
    std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager>,
    std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface>>;

/**
 * Get the manufactory @c Component for (legacy) @c DefaultClient initialization.
 *
 * @return The manufactory @c Component for (legacy) @c DefaultClient initialization.
 */
SmartScreenClientComponent getComponent(
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface>& authDelegate,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface>& contextManager,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& localeAssetsManager,
    const std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo>& deviceInfo,
    const std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager>& customerDataManager,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface>& miscStorage,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::InternetConnectionMonitorInterface>&
        internetConnectionMonitor,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayManagerInterface>& avsGatewayManager,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface>&
        capabilitiesDelegate,
    const std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface>& metricRecorder,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::diagnostics::DiagnosticsInterface>& diagnostics,
    const std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface>& transportFactory,
    const std::shared_ptr<alexaClientSDK::acl::MessageRouterFactoryInterface>& messageRouterFactory,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeFactoryInterface>&
        channelVolumeFactory,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExpectSpeechTimeoutHandlerInterface>&
        expectSpeechTimeoutHandler,
    const std::shared_ptr<alexaClientSDK::acsdkEqualizerInterfaces::EqualizerRuntimeSetupInterface>&
        equalizerRuntimeSetup,
    const std::shared_ptr<StubApplicationAudioPipelineFactory>& stubAudioPipelineFactory,
    const std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::PooledMediaResourceProviderInterface>&
        audioMediaResourceProvider,
    const std::shared_ptr<alexaClientSDK::certifiedSender::MessageStorageInterface>& messageStorage,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PowerResourceManagerInterface>&
        powerResourceManager,
    const alexaClientSDK::acsdkExternalMediaPlayer::ExternalMediaPlayer::AdapterCreationMap& adapterCreationMap,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SystemTimeZoneInterface>& systemTimeZone,
    const std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface>& deviceSettingStorage,
    bool startAlertSchedulingOnInitialization,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AudioFactoryInterface>& audioFactory,
    const std::shared_ptr<alexaClientSDK::acsdkAlerts::storage::AlertStorageInterface>& alertStorage);

}  // namespace smartScreenClient
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_SMARTSCREENCLIENT_INCLUDE_SMARTSCREENCLIENT_DEFAULTCLIENTCOMPONENT_H_
