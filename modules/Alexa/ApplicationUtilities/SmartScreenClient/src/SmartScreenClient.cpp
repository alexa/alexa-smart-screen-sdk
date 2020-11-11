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

#include <acsdkExternalMediaPlayerInterfaces/ExternalMediaAdapterConstants.h>
#include <ADSL/MessageInterpreter.h>
#include <AVSCommon/AVS/Attachment/AttachmentManager.h>
#include <AVSCommon/AVS/ExceptionEncounteredSender.h>
#include <AVSCommon/AVS/SpeakerConstants/SpeakerConstants.h>
#include <AVSCommon/SDKInterfaces/InternetConnectionMonitorInterface.h>
#include <AVSCommon/SDKInterfaces/SystemClockMonitorObserverInterface.h>
#include <AVSCommon/Utils/Bluetooth/BluetoothEventBus.h>
#include <AVSCommon/Utils/MediaPlayer/PooledMediaResourceProvider.h>
#include <AVSCommon/Utils/Metrics/MetricRecorderInterface.h>
#include <AVSCommon/Utils/Network/InternetConnectionMonitor.h>
#include <Audio/SystemSoundAudioFactory.h>
#include <Endpoints/EndpointBuilder.h>
#include <InterruptModel/InterruptModel.h>
#include <System/LocaleHandler.h>
#include <System/ReportStateHandler.h>
#include <System/SystemCapabilityProvider.h>
#include <System/TimeZoneHandler.h>
#include <System/UserInactivityMonitor.h>
#include <SystemSoundPlayer/SystemSoundPlayer.h>

#ifdef ENABLE_OPUS
#include <SpeechEncoder/OpusEncoderContext.h>
#endif

#ifdef ENABLE_PCC
#include <AVSCommon/SDKInterfaces/Phone/PhoneCallerInterface.h>
#include <PhoneCallController/PhoneCallController.h>
#endif

#ifdef ENABLE_MCC
#include <AVSCommon/SDKInterfaces/Calendar/CalendarClientInterface.h>
#include <AVSCommon/SDKInterfaces/Meeting/MeetingClientInterface.h>
#include <MeetingClientController/MeetingClientController.h>
#endif

#ifdef BLUETOOTH_BLUEZ
#include <BlueZ/BlueZBluetoothDeviceManager.h>
#include <acsdkBluetooth/BluetoothMediaInputTransformer.h>
#endif

#include <SDKComponent/SDKComponent.h>

#include "SmartScreenClient/DefaultClientComponent.h"
#include "SmartScreenClient/DeviceSettingsManagerBuilder.h"
#include "SmartScreenClient/StubApplicationAudioPipelineFactory.h"
#include "SmartScreenClient/SmartScreenClient.h"

namespace alexaSmartScreenSDK {
namespace smartScreenClient {

using namespace alexaClientSDK;
using namespace alexaClientSDK::acsdkApplicationAudioPipelineFactoryInterfaces;
using namespace alexaClientSDK::avsCommon;
using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::sdkInterfaces::endpoints;
using namespace alexaClientSDK::endpoints;

using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils;

/// String to identify log entries originating from this file.
static const std::string TAG("SmartScreenClient");

/// Key for visual channel array configurations in configuration node.
static const std::string VISUAL_CHANNEL_CONFIG_KEY = "visualChannels";

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::unique_ptr<SmartScreenClient> SmartScreenClient::create(
    const std::shared_ptr<SmartScreenClientManufactory>& manufactory,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> alertsMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> bluetoothMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> ringtoneMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> systemSoundMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> alertsSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> bluetoothSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> ringtoneSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> systemSoundSpeaker,
    const std::multimap<
        avsCommon::sdkInterfaces::ChannelVolumeInterface::Type,
        std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> additionalSpeakers,
#ifdef ENABLE_PCC
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> phoneSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::phone::PhoneCallerInterface> phoneCaller,
#endif
#ifdef ENABLE_MCC
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> meetingSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::meeting::MeetingClientInterface> meetingClient,
    std::shared_ptr<avsCommon::sdkInterfaces::calendar::CalendarClientInterface> calendarClient,
#endif
#ifdef ENABLE_COMMS_AUDIO_PROXY
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> commsMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> commsSpeaker,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> sharedDataStream,
#endif
    std::shared_ptr<avsCommon::sdkInterfaces::audio::AudioFactoryInterface> audioFactory,
    std::shared_ptr<acsdkAlerts::storage::AlertStorageInterface> alertStorage,
    std::shared_ptr<acsdkNotificationsInterfaces::NotificationsStorageInterface> notificationsStorage,
    std::unique_ptr<settings::storage::DeviceSettingStorageInterface> deviceSettingStorage,
    std::shared_ptr<acsdkBluetooth::BluetoothStorageInterface> bluetoothStorage,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface>>
        alexaDialogStateObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>
        connectionObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceConnectionRuleInterface>>
        enabledConnectionRules,
    std::shared_ptr<avsCommon::sdkInterfaces::SystemTimeZoneInterface> systemTimezone,
    avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion,
    bool sendSoftwareInfoOnConnected,
    std::shared_ptr<avsCommon::sdkInterfaces::SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver,
    std::unique_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager,
    std::shared_ptr<avsCommon::sdkInterfaces::diagnostics::DiagnosticsInterface> diagnostics,
    const std::shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder,
    bool startAlertSchedulingOnInitialization,
    capabilityAgents::aip::AudioProvider firstInteractionAudioProvider,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider,
    const std::string& APLMaxVersion) {
    std::unique_ptr<SmartScreenClient> smartScreenClient(new SmartScreenClient());
    if (!smartScreenClient->initialize(
            manufactory,
            alertsMediaPlayer,
            bluetoothMediaPlayer,
            ringtoneMediaPlayer,
            systemSoundMediaPlayer,
            alertsSpeaker,
            bluetoothSpeaker,
            ringtoneSpeaker,
            systemSoundSpeaker,
            additionalSpeakers,
#ifdef ENABLE_PCC
            phoneSpeaker,
            phoneCaller,
#endif
#ifdef ENABLE_MCC
            meetingSpeaker,
            meetingClient,
            calendarClient,
#endif
#ifdef ENABLE_COMMS_AUDIO_PROXY
            commsMediaPlayer,
            commsSpeaker,
            sharedDataStream,
#endif
            audioFactory,
            alertStorage,
            notificationsStorage,
            std::move(deviceSettingStorage),
            bluetoothStorage,
            alexaDialogStateObservers,
            connectionObservers,
            enabledConnectionRules,
            systemTimezone,
            firmwareVersion,
            sendSoftwareInfoOnConnected,
            softwareInfoSenderObserver,
            std::move(bluetoothDeviceManager),
            diagnostics,
            externalCapabilitiesBuilder,
            startAlertSchedulingOnInitialization,
            firstInteractionAudioProvider,
            visualStateProvider,
            APLMaxVersion)) {
        return nullptr;
    }
    return smartScreenClient;
}

std::unique_ptr<SmartScreenClient> SmartScreenClient::create(
    std::shared_ptr<avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<registrationManager::CustomerDataManager> customerDataManager,
    const std::unordered_map<std::string, std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>>&
        externalMusicProviderMediaPlayers,
    const std::unordered_map<std::string, std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>>&
        externalMusicProviderSpeakers,
    const acsdkExternalMediaPlayer::ExternalMediaPlayer::AdapterCreationMap& adapterCreationMap,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> speakMediaPlayer,
    std::unique_ptr<mediaPlayer::MediaPlayerFactoryInterface> audioMediaPlayerFactory,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> alertsMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> notificationsMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> bluetoothMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> ringtoneMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> systemSoundMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> speakSpeaker,
    std::vector<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> audioSpeakers,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> alertsSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> notificationsSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> bluetoothSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> ringtoneSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> systemSoundSpeaker,
    const std::multimap<
        avsCommon::sdkInterfaces::ChannelVolumeInterface::Type,
        std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> additionalSpeakers,
#ifdef ENABLE_PCC
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> phoneSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::phone::PhoneCallerInterface> phoneCaller,
#endif
#ifdef ENABLE_MCC
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> meetingSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::meeting::MeetingClientInterface> meetingClient,
    std::shared_ptr<avsCommon::sdkInterfaces::calendar::CalendarClientInterface> calendarClient,
#endif
#ifdef ENABLE_COMMS_AUDIO_PROXY
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> commsMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> commsSpeaker,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> sharedDataStream,
#endif
    std::shared_ptr<EqualizerRuntimeSetup> equalizerRuntimeSetup,
    std::shared_ptr<avsCommon::sdkInterfaces::audio::AudioFactoryInterface> audioFactory,
    std::shared_ptr<avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<acsdkAlerts::storage::AlertStorageInterface> alertStorage,
    std::shared_ptr<certifiedSender::MessageStorageInterface> messageStorage,
    std::shared_ptr<acsdkNotificationsInterfaces::NotificationsStorageInterface> notificationsStorage,
    std::unique_ptr<settings::storage::DeviceSettingStorageInterface> deviceSettingStorage,
    std::shared_ptr<acsdkBluetooth::BluetoothStorageInterface> bluetoothStorage,
    std::shared_ptr<avsCommon::sdkInterfaces::storage::MiscStorageInterface> miscStorage,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface>>
        alexaDialogStateObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>
        connectionObservers,
    std::shared_ptr<avsCommon::sdkInterfaces::InternetConnectionMonitorInterface> internetConnectionMonitor,
    std::shared_ptr<avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> transportFactory,
    std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayManagerInterface> avsGatewayManager,
    std::shared_ptr<avsCommon::sdkInterfaces::LocaleAssetsManagerInterface> localeAssetsManager,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceConnectionRuleInterface>>
        enabledConnectionRules,
    std::shared_ptr<avsCommon::sdkInterfaces::SystemTimeZoneInterface> systemTimezone,
    avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion,
    bool sendSoftwareInfoOnConnected,
    std::shared_ptr<avsCommon::sdkInterfaces::SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver,
    std::unique_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager,
    std::shared_ptr<avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder,
    std::shared_ptr<avsCommon::sdkInterfaces::PowerResourceManagerInterface> powerResourceManager,
    std::shared_ptr<avsCommon::sdkInterfaces::diagnostics::DiagnosticsInterface> diagnostics,
    const std::shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder,
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelVolumeFactoryInterface> channelVolumeFactory,
    bool startAlertSchedulingOnInitialization,
    std::shared_ptr<alexaClientSDK::acl::MessageRouterFactoryInterface> messageRouterFactory,
    const std::shared_ptr<avsCommon::sdkInterfaces::ExpectSpeechTimeoutHandlerInterface>& expectSpeechTimeoutHandler,
    capabilityAgents::aip::AudioProvider firstInteractionAudioProvider,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider,
    const std::string& APLMaxVersion) {

    if (!equalizerRuntimeSetup) {
        equalizerRuntimeSetup = std::make_shared<EqualizerRuntimeSetup>(false);
    }

    auto stubAudioPipelineFactory = StubApplicationAudioPipelineFactory::create(channelVolumeFactory);
    if (!stubAudioPipelineFactory) {
        ACSDK_ERROR(LX("createFailed").d("reason", "failed to create audio pipeline"));
        return nullptr;
    }

    /// Add pre-created speakers and media players to the stub factory.
    stubAudioPipelineFactory->addApplicationMediaInterfaces(
        acsdkNotifications::NOTIFICATIONS_MEDIA_PLAYER_NAME, notificationsMediaPlayer, notificationsSpeaker);
    stubAudioPipelineFactory->addApplicationMediaInterfaces(
        capabilityAgents::speechSynthesizer::SPEAK_MEDIA_PLAYER_NAME, speakMediaPlayer, speakSpeaker);
    for (const auto& adapter : adapterCreationMap) {
        auto mediaPlayerIt = externalMusicProviderMediaPlayers.find(adapter.first);
        auto speakerIt = externalMusicProviderSpeakers.find(adapter.first);

        if (mediaPlayerIt == externalMusicProviderMediaPlayers.end()) {
            ACSDK_ERROR(LX("externalMediaAdapterCreationFailed")
                            .d(acsdkExternalMediaPlayerInterfaces::PLAYER_ID, adapter.first)
                            .d("reason", "nullMediaPlayer"));
            continue;
        }

        if (speakerIt == externalMusicProviderSpeakers.end()) {
            ACSDK_ERROR(LX("externalMediaAdapterCreationFailed")
                            .d(acsdkExternalMediaPlayerInterfaces::PLAYER_ID, adapter.first)
                            .d("reason", "nullSpeaker"));
            continue;
        }

        stubAudioPipelineFactory->addApplicationMediaInterfaces(
            adapter.first + "MediaPlayer", (*mediaPlayerIt).second, (*speakerIt).second);
    }

    std::vector<std::shared_ptr<avsCommon::sdkInterfaces::ChannelVolumeInterface>> audioChannelVolumeInterfaces;
    for (auto& it : audioSpeakers) {
        audioChannelVolumeInterfaces.push_back(channelVolumeFactory->createChannelVolumeInterface(it));
    }
    auto audioMediaPlayerFactoryAdapter = mediaPlayer::PooledMediaResourceProvider::adaptMediaPlayerFactoryInterface(
        std::move(audioMediaPlayerFactory), audioChannelVolumeInterfaces);

    auto component = getComponent(
        authDelegate,
        contextManager,
        localeAssetsManager,
        deviceInfo,
        customerDataManager,
        miscStorage,
        internetConnectionMonitor,
        avsGatewayManager,
        capabilitiesDelegate,
        metricRecorder,
        diagnostics,
        transportFactory,
        messageRouterFactory,
        channelVolumeFactory,
        expectSpeechTimeoutHandler,
        equalizerRuntimeSetup,
        stubAudioPipelineFactory,
        audioMediaPlayerFactoryAdapter,
        messageStorage,
        powerResourceManager,
        adapterCreationMap);
    auto manufactory = SmartScreenClientManufactory::create(component);

    auto speakerManager = manufactory->get<std::shared_ptr<SpeakerManagerInterface>>();
    if (!speakerManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullSpeakerManager"));
        return nullptr;
    }
    for (const auto& it : audioChannelVolumeInterfaces) {
        speakerManager->addChannelVolumeInterface(it);
    }

    auto startupManager = manufactory->get<std::shared_ptr<acsdkStartupManagerInterfaces::StartupManagerInterface>>();
    if (!startupManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullStartupManager"));
        return nullptr;
    }
    startupManager->startup();

    return create(
        std::move(manufactory),
        alertsMediaPlayer,
        bluetoothMediaPlayer,
        ringtoneMediaPlayer,
        systemSoundMediaPlayer,
        alertsSpeaker,
        bluetoothSpeaker,
        ringtoneSpeaker,
        systemSoundSpeaker,
        additionalSpeakers,
#ifdef ENABLE_PCC
        phoneSpeaker,
        phoneCaller,
#endif
#ifdef ENABLE_MCC
        meetingSpeaker,
        meetingClient,
        calendarClient,
#endif
#ifdef ENABLE_COMMS_AUDIO_PROXY
        commsMediaPlayer,
        commsSpeaker,
        sharedDataStream,
#endif
        audioFactory,
        alertStorage,
        notificationsStorage,
        std::move(deviceSettingStorage),
        bluetoothStorage,
        alexaDialogStateObservers,
        connectionObservers,
        enabledConnectionRules,
        systemTimezone,
        firmwareVersion,
        sendSoftwareInfoOnConnected,
        softwareInfoSenderObserver,
        std::move(bluetoothDeviceManager),
        diagnostics,
        externalCapabilitiesBuilder,
        startAlertSchedulingOnInitialization,
        firstInteractionAudioProvider,
        visualStateProvider,
        APLMaxVersion);
}

bool SmartScreenClient::initialize(
    const std::shared_ptr<SmartScreenClientManufactory>& manufactory,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> alertsMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> bluetoothMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> ringtoneMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> systemSoundMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> alertsSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> bluetoothSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> ringtoneSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> systemSoundSpeaker,
    const std::multimap<
        avsCommon::sdkInterfaces::ChannelVolumeInterface::Type,
        std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> additionalSpeakers,
#ifdef ENABLE_PCC
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> phoneSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::phone::PhoneCallerInterface> phoneCaller,
#endif
#ifdef ENABLE_MCC
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> meetingSpeaker,
    std::shared_ptr<avsCommon::sdkInterfaces::meeting::MeetingClientInterface> meetingClient,
    std::shared_ptr<avsCommon::sdkInterfaces::calendar::CalendarClientInterface> calendarClient,
#endif
#ifdef ENABLE_COMMS_AUDIO_PROXY
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> commsMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface> commsSpeaker,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> sharedDataStream,
#endif
    std::shared_ptr<avsCommon::sdkInterfaces::audio::AudioFactoryInterface> audioFactory,
    std::shared_ptr<acsdkAlerts::storage::AlertStorageInterface> alertStorage,
    std::shared_ptr<acsdkNotificationsInterfaces::NotificationsStorageInterface> notificationsStorage,
    std::shared_ptr<settings::storage::DeviceSettingStorageInterface> deviceSettingStorage,
    std::shared_ptr<acsdkBluetooth::BluetoothStorageInterface> bluetoothStorage,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface>>
        alexaDialogStateObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>
        connectionObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceConnectionRuleInterface>>
        enabledConnectionRules,
    std::shared_ptr<avsCommon::sdkInterfaces::SystemTimeZoneInterface> systemTimezone,
    avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion,
    bool sendSoftwareInfoOnConnected,
    std::shared_ptr<avsCommon::sdkInterfaces::SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver,
    std::unique_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager,
    std::shared_ptr<avsCommon::sdkInterfaces::diagnostics::DiagnosticsInterface> diagnostics,
    const std::shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder,
    bool startAlertSchedulingOnInitialization,
    capabilityAgents::aip::AudioProvider firstInteractionAudioProvider,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider,
    const std::string& APLMaxVersion) {

    if (!audioFactory) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioFactory"));
        return false;
    }

    if (!alertsMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAlertsMediaPlayer"));
        return false;
    }

    if (!bluetoothMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullBluetoothMediaPlayer"));
        return false;
    }

    if (!ringtoneMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullRingtoneMediaPlayer"));
        return false;
    }

    if (!systemSoundMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSystemSoundMediaPlayer"));
        return false;
    }

    if (!deviceSettingStorage) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDeviceSettingStorage"));
        return false;
    }

    // Initialize various locals from manufactory.
    auto metricRecorder = manufactory->get<std::shared_ptr<avsCommon::utils::metrics::MetricRecorderInterface>>();
    if (!metricRecorder) {
        ACSDK_DEBUG7(LX(__func__).m("metrics disabled"));
    }

    auto customerDataManager = manufactory->get<std::shared_ptr<registrationManager::CustomerDataManager>>();
    if (!customerDataManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAttachmentManager"));
        return false;
    }

    auto attachmentManager =
        manufactory->get<std::shared_ptr<avsCommon::avs::attachment::AttachmentManagerInterface>>();
    if (!attachmentManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDefaultEndpointBuilder"));
        return false;
    }

    auto localeAssetsManager =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>>();
    if (!localeAssetsManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullLocaleAssetsManager"));
        return false;
    }

    auto miscStorage = manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::storage::MiscStorageInterface>>();
    if (!miscStorage) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullMiscStorage"));
        return false;
    }

    auto channelVolumeFactory =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::ChannelVolumeFactoryInterface>>();
    if (!channelVolumeFactory) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullChannelVolumeFactory"));
        return false;
    }

    auto audioPipelineFactory = manufactory->get<std::shared_ptr<ApplicationAudioPipelineFactoryInterface>>();
    if (!audioPipelineFactory) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioPipelineFactory"));
        return false;
    }

    if (!visualStateProvider) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullvisualStateProvider"));
        return false;
    }

    if (APLMaxVersion.empty()) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "emptyAPLVersion"));
        return false;
    }

    auto powerResourceManager =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::PowerResourceManagerInterface>>();
    if (!powerResourceManager) {
        ACSDK_DEBUG7(LX(__func__).m("power resource management disabled"));
    }

    // Initialize various members from manufactory.
    m_avsGatewayManager = manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayManagerInterface>>();
    if (!m_avsGatewayManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAVSGatewayManager"));
        return false;
    }

    m_internetConnectionMonitor =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::InternetConnectionMonitorInterface>>();
    if (!m_internetConnectionMonitor) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullConnectionManager"));
        return false;
    }

    m_connectionManager = manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::AVSConnectionManagerInterface>>();
    if (!m_connectionManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDefaultEndpointBuilder"));
        return false;
    }

    m_contextManager = manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface>>();
    if (!m_contextManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullContextManager"));
        return false;
    }

    m_capabilitiesDelegate =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::CapabilitiesDelegateInterface>>();
    if (!m_capabilitiesDelegate) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullCapabilitiesDelegate"));
        return false;
    }

    m_deviceInfo = manufactory->get<std::shared_ptr<avsCommon::utils::DeviceInfo>>();
    if (!m_deviceInfo) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDeviceInfo"));
        return false;
    }

    m_authDelegate = manufactory->get<std::shared_ptr<AuthDelegateInterface>>();
    if (!m_authDelegate) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAuthDelegate"));
        return false;
    }

    m_exceptionSender =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>>();
    if (!m_exceptionSender) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullExceptionSender"));
        return false;
    }

    m_alexaMessageSender = manufactory->get<std::shared_ptr<capabilityAgents::alexa::AlexaInterfaceMessageSender>>();
    if (!m_alexaMessageSender) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAlexaMessageSender"));
        return false;
    }

    m_speakerManager = manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerManagerInterface>>();
    if (!m_speakerManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSpeakerManager"));
        return false;
    }

    m_defaultEndpointBuilder = manufactory->get<acsdkManufactory::Annotated<
        DefaultEndpointAnnotation,
        avsCommon::sdkInterfaces::endpoints::EndpointBuilderInterface>>();
    if (!m_defaultEndpointBuilder) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDefaultEndpointBuilder"));
        return false;
    }

    m_captionManager = manufactory->get<std::shared_ptr<captions::CaptionManagerInterface>>();
    if (!m_captionManager) {
        ACSDK_DEBUG5(LX("nullCaptionManager").m("captions disabled"));
    }

    m_equalizerRuntimeSetup =
        manufactory->get<std::shared_ptr<acsdkEqualizerInterfaces::EqualizerRuntimeSetupInterface>>();
    if (!m_equalizerRuntimeSetup) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullEqualizerRuntimeSetup"));
        return false;
    }

    m_audioFocusManager =
        manufactory
            ->get<acsdkManufactory::Annotated<avsCommon::sdkInterfaces::AudioFocusAnnotation, FocusManagerInterface>>();
    if (!m_audioFocusManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioFocusManager"));
        return false;
    }

    m_playbackRouter = manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::PlaybackRouterInterface>>();
    if (!m_playbackRouter) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullPlaybackRouter"));
        return false;
    }

    m_audioPlayer = manufactory->get<std::shared_ptr<acsdkAudioPlayerInterfaces::AudioPlayerInterface>>();
    if (!m_audioPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioPlayer"));
        return false;
    }

    m_shutdownManager = manufactory->get<std::shared_ptr<acsdkShutdownManagerInterfaces::ShutdownManagerInterface>>();
    if (!m_shutdownManager) {
        ACSDK_ERROR(LX("initializeFailed").m("Failed to get ShutdownManager!"));
    }

    m_certifiedSender = manufactory->get<std::shared_ptr<certifiedSender::CertifiedSender>>();
    if (!m_certifiedSender) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullCertifiedSender"));
        return false;
    }

    m_externalMediaPlayer =
        manufactory->get<std::shared_ptr<acsdkExternalMediaPlayerInterfaces::ExternalMediaPlayerInterface>>();
    if (!m_externalMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullExternalMediaPlayer"));
        return false;
    }

    m_dialogUXStateAggregator = std::make_shared<avsCommon::avs::DialogUXStateAggregator>(metricRecorder);

    m_softwareReporterCapabilityAgent =
        capabilityAgents::softwareComponentReporter::SoftwareComponentReporterCapabilityAgent::create();
    if (!m_softwareReporterCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSoftwareReporterCapabilityAgent"));
        return false;
    }

    if (!applicationUtilities::SDKComponent::SDKComponent::registerComponent(m_softwareReporterCapabilityAgent)) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToRegisterSDKComponent"));
        return false;
    }

    for (auto& observer : connectionObservers) {
        m_connectionManager->addConnectionStatusObserver(observer);
    }

    m_connectionManager->addMessageObserver(m_dialogUXStateAggregator);

    for (auto observer : alexaDialogStateObservers) {
        m_dialogUXStateAggregator->addObserver(observer);
    }

    m_connectionManager->addMessageObserver(m_dialogUXStateAggregator);

    /*
     * Creating the Directive Sequencer - This is the component that deals with
     * the sequencing and ordering of
     * directives sent from AVS and forwarding them along to the appropriate
     * Capability Agent that deals with
     * directives in that Namespace/Name.
     */
    m_directiveSequencer = adsl::DirectiveSequencer::create(m_exceptionSender, metricRecorder);
    if (!m_directiveSequencer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateDirectiveSequencer"));
        return false;
    }

    /*
     * Creating the Message Interpreter - This component takes care of converting
     * ACL messages to Directives for the
     * Directive Sequencer to process. This essentially "glues" together the ACL
     * and ADSL.
     */
    auto messageInterpreter = std::make_shared<adsl::MessageInterpreter>(
        m_exceptionSender, m_directiveSequencer, attachmentManager, metricRecorder);

    m_connectionManager->addMessageObserver(messageInterpreter);

    /*
     * Creating the Registration Manager - This component is responsible for
     * implementing any customer registration
     * operation such as login and logout
     */
    m_registrationManager = std::make_shared<registrationManager::RegistrationManager>(
        m_directiveSequencer, m_connectionManager, customerDataManager);

    // Create endpoint related objects.
    m_capabilitiesDelegate->setMessageSender(m_connectionManager);
    m_avsGatewayManager->addObserver(m_capabilitiesDelegate);
    addConnectionObserver(m_capabilitiesDelegate);
    m_endpointRegistrationManager = EndpointRegistrationManager::create(
        m_directiveSequencer, m_capabilitiesDelegate, m_deviceInfo->getDefaultEndpointId());
    if (!m_endpointRegistrationManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "endpointRegistrationManagerCreateFailed"));
        return false;
    }

    m_deviceSettingStorage = deviceSettingStorage;
    if (!m_deviceSettingStorage->open()) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "deviceSettingStorageOpenFailed"));
        return false;
    }

    /*
     * Creating the DoNotDisturb Capability Agent.
     *
     * TODO(ACSDK-2279): Keep this here till we can inject DND setting into the DND CA.
     */
    m_dndCapabilityAgent = capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent::create(
        m_exceptionSender, m_connectionManager, m_deviceSettingStorage);

    if (!m_dndCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateDNDCapabilityAgent"));
        return false;
    }

    addConnectionObserver(m_dndCapabilityAgent);

    DeviceSettingsManagerBuilder settingsManagerBuilder{
        m_deviceSettingStorage, m_connectionManager, m_connectionManager, customerDataManager};
    settingsManagerBuilder.withDoNotDisturbSetting(m_dndCapabilityAgent)
        .withAlarmVolumeRampSetting()
        .withWakeWordConfirmationSetting()
        .withSpeechConfirmationSetting()
        .withTimeZoneSetting(systemTimezone)
        .withNetworkInfoSetting();

    if (localeAssetsManager->getDefaultSupportedWakeWords().empty()) {
        settingsManagerBuilder.withLocaleSetting(localeAssetsManager);
    } else {
        settingsManagerBuilder.withLocaleAndWakeWordsSettings(localeAssetsManager);
    }

    m_deviceSettingsManager = settingsManagerBuilder.build();
    if (!m_deviceSettingsManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "createDeviceSettingsManagerFailed"));
        return false;
    }

    m_deviceTimeZoneOffset = calculateDeviceTimezoneOffset(
        settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::TIMEZONE>()->get());

    /*
     * Creating the User Inactivity Monitor - This component is responsibly for
     * updating AVS of user inactivity as
     * described in the System Interface of AVS.
     */
    m_userInactivityMonitor =
        capabilityAgents::system::UserInactivityMonitor::create(m_connectionManager, m_exceptionSender);
    if (!m_userInactivityMonitor) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateUserInactivityMonitor"));
        return false;
    }

    m_systemSoundPlayer = applicationUtilities::systemSoundPlayer::SystemSoundPlayer::create(
        systemSoundMediaPlayer, audioFactory->systemSounds());

    auto wakeWordConfirmationSetting =
        settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::WAKEWORD_CONFIRMATION>();
    auto speechConfirmationSetting =
        settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::SPEECH_CONFIRMATION>();
    auto wakeWordsSetting = settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::WAKE_WORDS>();

/*
 * Creating the Audio Input Processor - This component is the Capability Agent
 * that implements the SpeechRecognizer interface of AVS.
 */
#ifdef ENABLE_OPUS
    m_audioInputProcessor = capabilityAgents::aip::AudioInputProcessor::create(
        m_directiveSequencer,
        m_connectionManager,
        m_contextManager,
        m_audioFocusManager,
        m_dialogUXStateAggregator,
        m_exceptionSender,
        m_userInactivityMonitor,
        m_systemSoundPlayer,
        localeAssetsManager,
        wakeWordConfirmationSetting,
        speechConfirmationSetting,
        wakeWordsSetting,
        std::make_shared<speechencoder::SpeechEncoder>(std::make_shared<speechencoder::OpusEncoderContext>()),
        firstInteractionAudioProvider,
        powerResourceManager,
        metricRecorder,
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::ExpectSpeechTimeoutHandlerInterface>>());
#else
    m_audioInputProcessor = capabilityAgents::aip::AudioInputProcessor::create(
        m_directiveSequencer,
        m_connectionManager,
        m_contextManager,
        m_audioFocusManager,
        m_dialogUXStateAggregator,
        m_exceptionSender,
        m_userInactivityMonitor,
        m_systemSoundPlayer,
        localeAssetsManager,
        wakeWordConfirmationSetting,
        speechConfirmationSetting,
        wakeWordsSetting,
        nullptr,
        firstInteractionAudioProvider,
        powerResourceManager,
        metricRecorder,
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::ExpectSpeechTimeoutHandlerInterface>>());
#endif

    if (!m_audioInputProcessor) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAudioInputProcessor"));
        return false;
    }
    // when internet is disconnected during dialog, terminate dialog
    addInternetConnectionObserver(m_audioInputProcessor);

    m_audioInputProcessor->addObserver(m_dialogUXStateAggregator);

    m_connectionRetryTrigger = ConnectionRetryTrigger::create(m_connectionManager, m_audioInputProcessor);
    if (!m_connectionRetryTrigger) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateConnectionRetryTrigger"));
        return false;
    }

    /*
     * Creating the Speech Synthesizer - This component is the Capability Agent
     * that implements the SpeechSynthesizer
     * interface of AVS.
     */
    m_speechSynthesizer = capabilityAgents::speechSynthesizer::SpeechSynthesizer::createSpeechSynthesizer(
        audioPipelineFactory,
        m_connectionManager,
        m_audioFocusManager,
        m_contextManager,
        m_exceptionSender,
        metricRecorder,
        m_dialogUXStateAggregator,
        m_captionManager,
        powerResourceManager);

    if (!m_speechSynthesizer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSpeechSynthesizer"));
        return false;
    }

    m_speechSynthesizer->addObserver(m_dialogUXStateAggregator);

    // Adding speech synthesizer to the set of DialogChannelObserverInterfaces, this will be used to clear
    // the dialog channel
    addDialogChannelObserverInterface(m_speechSynthesizer);

    // create @c SpeakerInterfaces for each @c Type
    std::vector<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> allAvsSpeakers{systemSoundSpeaker};
    std::vector<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> allAlertSpeakers{alertsSpeaker};
    // parse additional Speakers into the right speaker list.
    for (const auto& it : additionalSpeakers) {
        if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == it.first) {
            allAvsSpeakers.push_back(it.second);
        } else if (ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME == it.first) {
            allAlertSpeakers.push_back(it.second);
        }
    }

#ifdef ENABLE_PCC
    allAvsSpeakers.push_back(phoneSpeaker);
#endif

#ifdef ENABLE_MCC
    allAvsSpeakers.push_back(meetingSpeaker);
#endif

#ifdef ENABLE_COMMS_AUDIO_PROXY
    allAvsSpeakers.push_back(commsSpeaker);
#endif

    // create @c ChannelVolumeInterface instances for all @c SpeakerInterface instances
    std::vector<std::shared_ptr<avsCommon::sdkInterfaces::ChannelVolumeInterface>> allAvsChannelVolumeInterfaces,
        allAlertChannelVolumeInterfaces, allChannelVolumeInterfaces;

    // create allAvsChannelVolumeInterfaces using allAvsSpeakers
    for (auto& it : allAvsSpeakers) {
        allAvsChannelVolumeInterfaces.push_back(channelVolumeFactory->createChannelVolumeInterface(it));
    }

    // create @c ChannelVolumeInterface for bluetoothSpeaker (later used by Bluetooth CapabilityAgent)
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelVolumeInterface> bluetoothChannelVolumeInterface =
        channelVolumeFactory->createChannelVolumeInterface(bluetoothSpeaker);
    allAvsChannelVolumeInterfaces.push_back(bluetoothChannelVolumeInterface);

    // create @c ChannelVolumeInterface for ringtoneSpeaker
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelVolumeInterface> ringtoneChannelVolumeInterface =
        channelVolumeFactory->createChannelVolumeInterface(ringtoneSpeaker);
    allAvsChannelVolumeInterfaces.push_back(ringtoneChannelVolumeInterface);

    // create @c ChannelVolumeInterface for allAlertSpeakers
    for (auto& it : allAlertSpeakers) {
        allAlertChannelVolumeInterfaces.push_back(
            channelVolumeFactory->createChannelVolumeInterface(it, ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME));
    }

    allChannelVolumeInterfaces.insert(
        allChannelVolumeInterfaces.end(), allAvsChannelVolumeInterfaces.begin(), allAvsChannelVolumeInterfaces.end());
    allChannelVolumeInterfaces.insert(
        allChannelVolumeInterfaces.end(),
        allAlertChannelVolumeInterfaces.begin(),
        allAlertChannelVolumeInterfaces.end());

    for (const auto& channelVolumeInterface : allChannelVolumeInterfaces) {
        m_speakerManager->addChannelVolumeInterface(channelVolumeInterface);
    }

    auto alertRenderer = acsdkAlerts::renderer::Renderer::create(alertsMediaPlayer, metricRecorder);
    if (!alertRenderer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlarmRenderer"));
        return false;
    }

    /*
     * Creating the Alerts Capability Agent - This component is the Capability
     * Agent that implements the Alerts interface of AVS.
     */
    m_alertsCapabilityAgent = acsdkAlerts::AlertsCapabilityAgent::create(
        m_connectionManager,
        m_connectionManager,
        m_certifiedSender,
        m_audioFocusManager,
        m_speakerManager,
        m_contextManager,
        m_exceptionSender,
        alertStorage,
        audioFactory->alerts(),
        alertRenderer,
        customerDataManager,
        settingsManagerBuilder.getSetting<settings::ALARM_VOLUME_RAMP>(),
        m_deviceSettingsManager,
        metricRecorder,
        startAlertSchedulingOnInitialization);

    if (!m_alertsCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlertsCapabilityAgent"));
        return false;
    }

    /*
     * Creating the System Clock Monitor - This component notifies the time sensitive
     * components when the system clock resynchronizes.
     */
    m_systemClockMonitor = std::shared_ptr<avsCommon::utils::timing::SystemClockMonitor>(
        new avsCommon::utils::timing::SystemClockMonitor());
    m_systemClockMonitor->addSystemClockMonitorObserver(m_alertsCapabilityAgent);

    addConnectionObserver(m_dialogUXStateAggregator);

    m_notificationsRenderer =
        acsdkNotifications::NotificationRenderer::create(audioPipelineFactory, m_audioFocusManager);
    if (!m_notificationsRenderer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateNotificationsRenderer"));
        return false;
    }

    /*
     * Creating the Notifications Capability Agent - This component is the
     * Capability Agent that implements the
     * Notifications interface of AVS.
     */
    m_notificationsCapabilityAgent = acsdkNotifications::NotificationsCapabilityAgent::create(
        notificationsStorage,
        m_notificationsRenderer,
        m_contextManager,
        m_exceptionSender,
        audioFactory->notifications(),
        customerDataManager,
        metricRecorder);
    if (!m_notificationsCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateNotificationsCapabilityAgent"));
        return false;
    }

    m_interactionCapabilityAgent = capabilityAgents::interactionModel::InteractionModelCapabilityAgent::create(
        m_directiveSequencer, m_exceptionSender);
    if (!m_interactionCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateInteractionModelCapabilityAgent"));
        return false;
    }
    // Listen to when Request Processing Started (RPS) directive is received
    // to enter the THINKING mode (Interaction Model 1.1).
    m_interactionCapabilityAgent->addObserver(m_dialogUXStateAggregator);

#ifdef ENABLE_PCC
    /*
     * Creating the PhoneCallController - This component is the Capability Agent
     * that implements the
     * PhoneCallController interface of AVS
     */
    m_phoneCallControllerCapabilityAgent = capabilityAgents::phoneCallController::PhoneCallController::create(
        contextManager, m_connectionManager, phoneCaller, phoneSpeaker, m_audioFocusManager, m_exceptionSender);
    if (!m_phoneCallControllerCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreatePhoneCallControllerCapabilityAgent"));
    }
#endif

#ifdef ENABLE_MCC
    /*
     * Creating the MeetingClientController - This component is the Capability Agent that implements the
     * MeetingClientController interface of AVS
     */
    m_meetingClientControllerCapabilityAgent =
        capabilityAgents::meetingClientController::MeetingClientController::create(
            contextManager,
            m_connectionManager,
            meetingClient,
            calendarClient,
            m_speakerManager,
            m_audioFocusManager,
            m_exceptionSender);
    if (!m_meetingClientControllerCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateMeetingClientControllerCapabilityAgent"));
    }
#endif

    /*
     * Creating the Visual Activity Tracker - This component is responsibly for
     * reporting the visual channel focus
     * information to AVS.
     */
    m_visualActivityTracker = afml::VisualActivityTracker::create(m_contextManager);

    // Read visualVirtualChannels from config file
    std::vector<afml::FocusManager::ChannelConfiguration> visualVirtualChannelConfiguration;
    if (!afml::FocusManager::ChannelConfiguration::readChannelConfiguration(
            VISUAL_CHANNEL_CONFIG_KEY, &visualVirtualChannelConfiguration)) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToReadVisualChannels"));
        return false;
    }

    auto interruptModel = manufactory->get<std::shared_ptr<afml::interruptModel::InterruptModel>>();

    /*
     * Creating the Visual Focus Manager - This component deals with the
     * management of visual focus across various
     * components. It handles granting access to Channels as well as pushing
     * different "Channels" to foreground,
     * background, or no focus based on which other Channels are active and the
     * priorities of those Channels. Each
     * Capability Agent will require the Focus Manager in order to request
     * access to the Channel it wishes to play
     * on.
     */
    m_visualFocusManager = std::make_shared<afml::FocusManager>(
        afml::FocusManager::getDefaultVisualChannels(),
        m_visualActivityTracker,
        visualVirtualChannelConfiguration,
        interruptModel);

    /*
     * Creating the AlexaPresentation Capability Agent - This component is the Capability Agent that
     * implements the AlexaPresentation and AlexaPresentation.APL AVS interface.
     */
    m_alexaPresentation =
        alexaSmartScreenSDK::smartScreenCapabilityAgents::alexaPresentation::AlexaPresentation::create(
            m_visualFocusManager,
            m_exceptionSender,
            metricRecorder,
            m_connectionManager,
            m_contextManager,
            visualStateProvider);
    if (!m_alexaPresentation) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlexaPresentationCapabilityAgent"));
        return false;
    }
    m_dialogUXStateAggregator->addObserver(m_alexaPresentation);
    m_alexaPresentation->setAPLMaxVersion(APLMaxVersion);

    auto renderPlayerInfoCardsProviderRegistrar =
        manufactory->get<std::shared_ptr<avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderRegistrarInterface>>();
    if (!renderPlayerInfoCardsProviderRegistrar) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullRenderPlayerInfoCardsProviderRegistrar"));
        return false;
    }

    /*
     * Creating the TemplateRuntime Capability Agent - This component is the
     * Capability Agent that implements the
     * TemplateRuntime interface of AVS.
     */
    m_templateRuntime =
        alexaSmartScreenSDK::smartScreenCapabilityAgents::templateRuntime::TemplateRuntime::createTemplateRuntime(
            renderPlayerInfoCardsProviderRegistrar, m_visualFocusManager, m_exceptionSender);
    if (!m_templateRuntime) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateTemplateRuntimeCapabilityAgent"));
        return false;
    }
    m_dialogUXStateAggregator->addObserver(m_templateRuntime);
    addAlexaPresentationObserver(m_templateRuntime);

    if (externalCapabilitiesBuilder) {
        externalCapabilitiesBuilder->withTemplateRunTime(m_templateRuntime);
    }

    /*
     * Creating the VisualCharacteristics Capability Agent - This component is the Capability Agent that
     * publish Alexa.Display, Alexa.Display.Window, Alexa.InteractionMode,Alexa.Presentation.APL.Video interfaces.
     */
    m_visualCharacteristics =
        alexaSmartScreenSDK::smartScreenCapabilityAgents::visualCharacteristics::VisualCharacteristics::create(
            m_contextManager);
    if (!m_visualCharacteristics) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateVisualCharacteristicsCapabilityAgent"));
        return false;
    }

    /*
     * Creating the Equalizer Capability Agent and related implementations if
     * enabled
     */
    if (m_equalizerRuntimeSetup->isEnabled()) {
        auto equalizerController = acsdkEqualizer::EqualizerController::create(
            m_equalizerRuntimeSetup->getModeController(),
            m_equalizerRuntimeSetup->getConfiguration(),
            m_equalizerRuntimeSetup->getStorage());

        if (!equalizerController) {
            ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateEqualizerController"));
            return false;
        }

        m_equalizerCapabilityAgent = acsdkEqualizer::EqualizerCapabilityAgent::create(
            equalizerController,
            m_capabilitiesDelegate,
            m_equalizerRuntimeSetup->getStorage(),
            customerDataManager,
            m_exceptionSender,
            m_contextManager,
            m_connectionManager);
        if (!m_equalizerCapabilityAgent) {
            ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateEqualizerCapabilityAgent"));
            return false;
        }

        m_equalizerController = equalizerController;
        // Register equalizers
        for (auto& equalizer : m_equalizerRuntimeSetup->getAllEqualizers()) {
            equalizerController->registerEqualizer(equalizer);
        }

        // Add all equalizer controller listeners
        for (auto& listener : m_equalizerRuntimeSetup->getAllEqualizerControllerListeners()) {
            equalizerController->addListener(listener);
        }
    } else {
        ACSDK_DEBUG3(LX(__func__).m("Equalizer is disabled"));
    }

    /*
     * Creating the TimeZone Handler - This component is responsible for handling directives related to time zones.
     */
    auto timezoneHandler = capabilityAgents::system::TimeZoneHandler::create(
        settingsManagerBuilder.getSetting<settings::TIMEZONE>(), m_exceptionSender);
    if (!timezoneHandler) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateTimeZoneHandler"));
        return false;
    }

    /*
     * Creating the Locale Handler - This component is responsible for handling directives related to locales.
     */
    auto localeHandler = capabilityAgents::system::LocaleHandler::create(
        m_exceptionSender, settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::LOCALE>());
    if (!localeHandler) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateLocaleHandler"));
        return false;
    }

    /*
     * Creating the ReportState Handler - This component is responsible for the ReportState directives.
     */
    auto reportGenerator = capabilityAgents::system::StateReportGenerator::create(
        m_deviceSettingsManager, settingsManagerBuilder.getConfigurations());
    if (!reportGenerator.hasValue()) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateStateReportGenerator"));
        return false;
    }

    std::vector<capabilityAgents::system::StateReportGenerator> reportGenerators{{reportGenerator.value()}};
    std::shared_ptr<capabilityAgents::system::ReportStateHandler> reportStateHandler =
        capabilityAgents::system::ReportStateHandler::create(
            customerDataManager,
            m_exceptionSender,
            m_connectionManager,
            m_connectionManager,
            miscStorage,
            reportGenerators);
    if (!reportStateHandler) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateReportStateHandler"));
        return false;
    }

    /*
     * Creating the SystemCapabilityProvider - This component is responsible for
     * publishing information about the System
     * capability agent.
     */
    auto systemCapabilityProvider = capabilityAgents::system::SystemCapabilityProvider::create(localeAssetsManager);
    if (!systemCapabilityProvider) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSystemCapabilityProvider"));
        return false;
    }

#ifdef ENABLE_REVOKE_AUTH
    /*
     * Creating the RevokeAuthorizationHandler - This component is responsible for
     * handling RevokeAuthorization
     * directives from AVS to notify the client to clear out authorization and
     * re-enter the registration flow.
     */
    m_revokeAuthorizationHandler = capabilityAgents::system::RevokeAuthorizationHandler::create(m_exceptionSender);
    if (!m_revokeAuthorizationHandler) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateRevokeAuthorizationHandler"));
        return false;
    }
#endif

    if (bluetoothDeviceManager) {
        ACSDK_DEBUG5(LX(__func__).m("Creating Bluetooth CA"));

        // Create a temporary pointer to the eventBus inside of
        // bluetoothDeviceManager so that
        // the unique ptr for bluetoothDeviceManager can be moved.
        auto eventBus = bluetoothDeviceManager->getEventBus();

        auto bluetoothMediaInputTransformer =
            acsdkBluetooth::BluetoothMediaInputTransformer::create(eventBus, m_playbackRouter);

        /*
         * Creating the Bluetooth Capability Agent - This component is responsible
         * for handling directives from AVS
         * regarding bluetooth functionality.
         */
        m_bluetooth = acsdkBluetooth::Bluetooth::create(
            m_contextManager,
            m_audioFocusManager,
            m_connectionManager,
            m_exceptionSender,
            std::move(bluetoothStorage),
            std::move(bluetoothDeviceManager),
            std::move(eventBus),
            bluetoothMediaPlayer,
            customerDataManager,
            enabledConnectionRules,
            bluetoothChannelVolumeInterface,
            bluetoothMediaInputTransformer);
    } else {
        ACSDK_DEBUG5(LX("bluetoothCapabilityAgentDisabled").d("reason", "nullBluetoothDeviceManager"));
    }

    m_apiGatewayCapabilityAgent =
        capabilityAgents::apiGateway::ApiGatewayCapabilityAgent::create(m_avsGatewayManager, m_exceptionSender);
    if (!m_apiGatewayCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateApiGatewayCapabilityAgent"));
    }

    /**
     * Optional DiagnosticsInterface which provides diagnostic insights into the SDK.
     */
    m_diagnostics = diagnostics;
    if (m_diagnostics) {
        m_diagnostics->setDiagnosticDependencies(m_directiveSequencer, attachmentManager, m_connectionManager);

        /**
         * Create and initialize the DevicePropertyAggregator.
         */
        auto deviceProperties = m_diagnostics->getDevicePropertyAggregator();
        if (deviceProperties) {
            deviceProperties->setContextManager(m_contextManager);
            deviceProperties->initializeVolume(m_speakerManager);
            deviceProperties->setDeviceSettingsManager(m_deviceSettingsManager);
            addSpeakerManagerObserver(deviceProperties);
            addAlertsObserver(deviceProperties);
            addConnectionObserver(deviceProperties);
            addNotificationsObserver(deviceProperties);
            addAudioPlayerObserver(deviceProperties);
            addAlexaDialogStateObserver(deviceProperties);
            m_authDelegate->addAuthObserver(deviceProperties);
        }

        /**
         * Initialize device protocol tracer.
         */
        auto protocolTrace = m_diagnostics->getProtocolTracer();
        if (protocolTrace) {
            addMessageObserver(protocolTrace);
        }
    } else {
        ACSDK_DEBUG0(LX(__func__).m("Diagnostics Not Enabled"));
    }

    /*
     * Register capability agents and capability configurations.
     */
    m_defaultEndpointBuilder->withCapability(m_speechSynthesizer, m_speechSynthesizer);
    m_defaultEndpointBuilder->withCapability(m_audioInputProcessor, m_audioInputProcessor);
    m_defaultEndpointBuilder->withCapability(m_alertsCapabilityAgent, m_alertsCapabilityAgent);
    m_defaultEndpointBuilder->withCapability(m_apiGatewayCapabilityAgent, m_apiGatewayCapabilityAgent);
#ifdef ENABLE_PCC
    if (m_phoneCallControllerCapabilityAgent) {
        m_defaultEndpointBuilder->withCapability(
            m_phoneCallControllerCapabilityAgent, m_phoneCallControllerCapabilityAgent);
    }
#endif

#ifdef ENABLE_MCC
    if (m_meetingClientControllerCapabilityAgent) {
        m_defaultEndpointBuilder->withCapability(
            m_meetingClientControllerCapabilityAgent, m_meetingClientControllerCapabilityAgent);
    }
#endif

    m_defaultEndpointBuilder->withCapability(m_alexaPresentation, m_alexaPresentation);
    m_defaultEndpointBuilder->withCapability(m_templateRuntime, m_templateRuntime);
    m_defaultEndpointBuilder->withCapabilityConfiguration(m_visualCharacteristics);
    m_defaultEndpointBuilder->withCapabilityConfiguration(m_visualActivityTracker);

    m_defaultEndpointBuilder->withCapability(m_notificationsCapabilityAgent, m_notificationsCapabilityAgent);
    m_defaultEndpointBuilder->withCapability(m_interactionCapabilityAgent, m_interactionCapabilityAgent);

    if (m_bluetooth) {
        m_defaultEndpointBuilder->withCapability(m_bluetooth, m_bluetooth);
    }

    if (m_equalizerCapabilityAgent) {
        m_defaultEndpointBuilder->withCapability(m_equalizerCapabilityAgent, m_equalizerCapabilityAgent);
    }

    m_defaultEndpointBuilder->withCapability(m_dndCapabilityAgent, m_dndCapabilityAgent);

    // System CA is split into multiple directive handlers.
    m_defaultEndpointBuilder->withCapabilityConfiguration(systemCapabilityProvider);
    if (!m_directiveSequencer->addDirectiveHandler(std::move(localeHandler)) ||
        !m_directiveSequencer->addDirectiveHandler(std::move(timezoneHandler)) ||
        !m_directiveSequencer->addDirectiveHandler(reportStateHandler) ||
#ifdef ENABLE_REVOKE_AUTH
        !m_directiveSequencer->addDirectiveHandler(m_revokeAuthorizationHandler) ||
#endif
        !m_directiveSequencer->addDirectiveHandler(m_userInactivityMonitor)) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToRegisterSystemDirectiveHandler"));
        return false;
    }

    if (externalCapabilitiesBuilder) {
        externalCapabilitiesBuilder->withSettingsStorage(m_deviceSettingStorage);
        externalCapabilitiesBuilder->withInternetConnectionMonitor(m_internetConnectionMonitor);
        externalCapabilitiesBuilder->withDialogUXStateAggregator(m_dialogUXStateAggregator);
        externalCapabilitiesBuilder->withVisualFocusManager(m_visualFocusManager);

        auto concreteExternalMediaPlayer =
            manufactory->get<std::shared_ptr<acsdkExternalMediaPlayer::ExternalMediaPlayer>>();
        auto externalCapabilities = externalCapabilitiesBuilder->buildCapabilities(
            concreteExternalMediaPlayer,
            m_connectionManager,
            m_connectionManager,
            m_exceptionSender,
            m_certifiedSender,
            m_audioFocusManager,
            customerDataManager,
            reportStateHandler,
            m_audioInputProcessor,
            m_speakerManager,
            m_directiveSequencer,
            m_userInactivityMonitor,
            m_contextManager,
            m_avsGatewayManager,
            ringtoneMediaPlayer,
            audioFactory,
            ringtoneChannelVolumeInterface,
#ifdef ENABLE_COMMS_AUDIO_PROXY
            commsMediaPlayer,
            commsSpeaker,
            sharedDataStream,
#endif
            powerResourceManager,
            m_softwareReporterCapabilityAgent);
        for (auto& capability : externalCapabilities.first) {
            if (capability.configuration.hasValue()) {
                m_defaultEndpointBuilder->withCapability(capability.configuration.value(), capability.directiveHandler);
            } else {
                m_directiveSequencer->addDirectiveHandler(capability.directiveHandler);
            }
        }
        m_shutdownObjects.insert(
            m_shutdownObjects.end(), externalCapabilities.second.begin(), externalCapabilities.second.end());
        m_callManager = externalCapabilitiesBuilder->getCallManager();
    }

    if (softwareInfoSenderObserver) {
        m_softwareInfoSenderObservers.insert(softwareInfoSenderObserver);
    }

    if (m_callManager) {
        m_softwareInfoSenderObservers.insert(m_callManager);
    }

    if (avsCommon::sdkInterfaces::softwareInfo::isValidFirmwareVersion(firmwareVersion)) {
        auto tempSender = capabilityAgents::system::SoftwareInfoSender::create(
            firmwareVersion,
            sendSoftwareInfoOnConnected,
            m_softwareInfoSenderObservers,
            m_connectionManager,
            m_connectionManager,
            m_exceptionSender);
        if (tempSender) {
            std::lock_guard<std::mutex> lock(m_softwareInfoSenderMutex);
            m_softwareInfoSender = tempSender;
        } else {
            ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSoftwareInfoSender"));
            return false;
        }
    }

    m_defaultEndpointBuilder->withCapabilityConfiguration(m_softwareReporterCapabilityAgent);
    return true;
}

void SmartScreenClient::connect(bool performReset) {
    if (performReset) {
        if (m_defaultEndpointBuilder) {
            // Build default endpoint.
            auto defaultEndpoint = m_defaultEndpointBuilder->build();
            if (!defaultEndpoint) {
                ACSDK_CRITICAL(LX("connectFailed").d("reason", "couldNotBuildDefaultEndpoint"));
                return;
            }

            // Register default endpoint. Only wait for immediate failures and return with a critical error, if so.
            // Otherwise, the default endpoint will be registered with AVS in the post-connect stage (once
            // m_connectionManager->enable() is called, below). We should not block on waiting for resultFuture
            // to be ready, since instead we rely on the post-connect operation and the onCapabilitiesStateChange
            // callback.
            auto resultFuture = m_endpointRegistrationManager->registerEndpoint(std::move(defaultEndpoint));
            if ((resultFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)) {
                auto result = resultFuture.get();
                if (result != alexaClientSDK::endpoints::EndpointRegistrationManager::RegistrationResult::SUCCEEDED) {
                    ACSDK_CRITICAL(LX("connectFailed").d("reason", "registrationFailed").d("result", result));
                    return;
                }
            }
            m_defaultEndpointBuilder.reset();
        }
        // Ensure default endpoint registration is enqueued with @c EndpointRegistrationManager
        // before proceeding with connection. Otherwise, we risk a race condition where the post-connect operations
        // are created before the default endpoint is enqueued for publishing to AVS.
        m_endpointRegistrationManager->waitForPendingRegistrationsToEnqueue();
        m_avsGatewayManager->setAVSGatewayAssigner(m_connectionManager);
    }
    m_connectionManager->enable();
}

void SmartScreenClient::disconnect() {
    m_connectionManager->disable();
}

std::string SmartScreenClient::getAVSGateway() {
    return m_connectionManager->getAVSGateway();
}

/// === Workaround start ===
/**
 * In order to support multi-turn interactions SDK processes SpeechSynthesizer audio context in special way. This
 * leads to skill context not been cleared on cloud side when we locally exit. In order to fix that we should grab
 * DIALOG channel by interface processed in normal way and proceed as before.
 * More global AVS C++ SDK solution to be implemented later.
 */
/// Interface name to use for focus requests.
static const std::string APL_INTERFACE("Alexa.Presentation.APL");

void SmartScreenClient::forceClearDialogChannelFocus() {
    ACSDK_DEBUG5(LX(__func__).m("Force Clear Dialog Channel"));
    m_audioFocusManager->acquireChannel(
            avsCommon::sdkInterfaces::FocusManagerInterface::DIALOG_CHANNEL_NAME, shared_from_this(), APL_INTERFACE);
}

void SmartScreenClient::onFocusChanged(
    alexaClientSDK::avsCommon::avs::FocusState newFocus,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    if (newFocus == alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND) {
        stopForegroundActivity();
        m_audioInputProcessor->resetState();
    }
}
/// === Workaround end ===

/**
 * This function is called when the user clicks on an APL card in response to an Expect Speech,
 * setting the state of AIP to IDLE
 */
void SmartScreenClient::onUserEvent(alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State state) {
    ACSDK_DEBUG0(LX(__func__).m(AudioInputProcessorObserverInterface::stateToString(state)));
    if(state == AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH){
        m_audioInputProcessor->resetState();
    }
}

void SmartScreenClient::forceExit() {
    ACSDK_DEBUG5(LX(__func__).m("Force Exit"));
    clearAllExecuteCommands();
    clearCard();
    stopAllActivities();
    forceClearDialogChannelFocus();
}

void SmartScreenClient::clearCard() {
    m_alexaPresentation->clearCard();
    m_templateRuntime->clearCard();
}

void SmartScreenClient::stopForegroundActivity() {
    m_audioFocusManager->stopForegroundActivity();
}

void SmartScreenClient::stopAllActivities() {
    m_audioFocusManager->stopAllActivities();
}

void SmartScreenClient::localStopActiveAlert() {
    m_alertsCapabilityAgent->onLocalStop();
}

void SmartScreenClient::addAlexaDialogStateObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface> observer) {
    m_dialogUXStateAggregator->addObserver(observer);
}

void SmartScreenClient::removeAlexaDialogStateObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface> observer) {
    m_dialogUXStateAggregator->removeObserver(observer);
}

void SmartScreenClient::addMessageObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer) {
    m_connectionManager->addMessageObserver(observer);
}

void SmartScreenClient::removeMessageObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer) {
    m_connectionManager->removeMessageObserver(observer);
}

void SmartScreenClient::addConnectionObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface> observer) {
    m_connectionManager->addConnectionStatusObserver(observer);
}

void SmartScreenClient::removeConnectionObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface> observer) {
    m_connectionManager->removeConnectionStatusObserver(observer);
}

void SmartScreenClient::addInternetConnectionObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::InternetConnectionObserverInterface> observer) {
    m_internetConnectionMonitor->addInternetConnectionObserver(observer);
}

void SmartScreenClient::removeInternetConnectionObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::InternetConnectionObserverInterface> observer) {
    m_internetConnectionMonitor->removeInternetConnectionObserver(observer);
}

void SmartScreenClient::addAlertsObserver(std::shared_ptr<acsdkAlertsInterfaces::AlertObserverInterface> observer) {
    m_alertsCapabilityAgent->addObserver(observer);
}

void SmartScreenClient::removeAlertsObserver(std::shared_ptr<acsdkAlertsInterfaces::AlertObserverInterface> observer) {
    m_alertsCapabilityAgent->removeObserver(observer);
}

void SmartScreenClient::addAudioPlayerObserver(
    std::shared_ptr<acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface> observer) {
    m_audioPlayer->addObserver(observer);
}

void SmartScreenClient::removeAudioPlayerObserver(
    std::shared_ptr<acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface> observer) {
    m_audioPlayer->removeObserver(observer);
}

void SmartScreenClient::addTemplateRuntimeObserver(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::TemplateRuntimeObserverInterface> observer) {
    m_templateRuntime->addObserver(observer);
}

void SmartScreenClient::removeTemplateRuntimeObserver(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::TemplateRuntimeObserverInterface> observer) {
    m_templateRuntime->removeObserver(observer);
}

void SmartScreenClient::TemplateRuntimeDisplayCardCleared() {
    m_templateRuntime->displayCardCleared();
}

void SmartScreenClient::addNotificationsObserver(
    std::shared_ptr<acsdkNotificationsInterfaces::NotificationsObserverInterface> observer) {
    m_notificationsCapabilityAgent->addObserver(observer);
}

void SmartScreenClient::removeNotificationsObserver(
    std::shared_ptr<acsdkNotificationsInterfaces::NotificationsObserverInterface> observer) {
    m_notificationsCapabilityAgent->removeObserver(observer);
}

void SmartScreenClient::addExternalMediaPlayerObserver(
    std::shared_ptr<acsdkExternalMediaPlayerInterfaces::ExternalMediaPlayerObserverInterface> observer) {
    m_externalMediaPlayer->addObserver(observer);
}

void SmartScreenClient::removeExternalMediaPlayerObserver(
    std::shared_ptr<acsdkExternalMediaPlayerInterfaces::ExternalMediaPlayerObserverInterface> observer) {
    m_externalMediaPlayer->removeObserver(observer);
}

void SmartScreenClient::addCaptionPresenter(std::shared_ptr<captions::CaptionPresenterInterface> presenter) {
    if (m_captionManager) {
        m_captionManager->setCaptionPresenter(presenter);
    }
}

void SmartScreenClient::setCaptionMediaPlayers(
    const std::vector<std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>>& mediaPlayers) {
    if (m_captionManager) {
        m_captionManager->setMediaPlayers(mediaPlayers);
    }
}

void SmartScreenClient::addBluetoothDeviceObserver(
    std::shared_ptr<acsdkBluetoothInterfaces::BluetoothDeviceObserverInterface> observer) {
    if (!m_bluetooth) {
        ACSDK_DEBUG5(LX(__func__).m("bluetooth is disabled, not adding observer"));
        return;
    }
    m_bluetooth->addObserver(observer);
}

void SmartScreenClient::removeBluetoothDeviceObserver(
    std::shared_ptr<acsdkBluetoothInterfaces::BluetoothDeviceObserverInterface> observer) {
    if (!m_bluetooth) {
        return;
    }
    m_bluetooth->removeObserver(observer);
}

#ifdef ENABLE_REVOKE_AUTH
void SmartScreenClient::addRevokeAuthorizationObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::RevokeAuthorizationObserverInterface> observer) {
    if (!m_revokeAuthorizationHandler) {
        ACSDK_ERROR(LX("addRevokeAuthorizationObserver").d("reason", "revokeAuthorizationNotSupported"));
        return;
    }
    m_revokeAuthorizationHandler->addObserver(observer);
}

void SmartScreenClient::removeRevokeAuthorizationObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::RevokeAuthorizationObserverInterface> observer) {
    if (!m_revokeAuthorizationHandler) {
        ACSDK_ERROR(LX("removeRevokeAuthorizationObserver").d("reason", "revokeAuthorizationNotSupported"));
        return;
    }
    m_revokeAuthorizationHandler->removeObserver(observer);
}
#endif

std::shared_ptr<settings::DeviceSettingsManager> SmartScreenClient::getSettingsManager() {
    return m_deviceSettingsManager;
}

std::shared_ptr<avsCommon::sdkInterfaces::PlaybackRouterInterface> SmartScreenClient::getPlaybackRouter() const {
    return m_playbackRouter;
}

std::shared_ptr<alexaSmartScreenSDK::smartScreenCapabilityAgents::alexaPresentation::AlexaPresentation>
SmartScreenClient::getAlexaPresentation() const {
    return m_alexaPresentation;
}

std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> SmartScreenClient::getAudioFocusManager() const {
    return m_audioFocusManager;
}

std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> SmartScreenClient::getVisualFocusManager() const {
    return m_visualFocusManager;
}

std::shared_ptr<registrationManager::RegistrationManager> SmartScreenClient::getRegistrationManager() {
    return m_registrationManager;
}

std::shared_ptr<acsdkEqualizer::EqualizerController> SmartScreenClient::getEqualizerController() {
    return m_equalizerController;
}

void SmartScreenClient::addEqualizerControllerListener(
    std::shared_ptr<acsdkEqualizerInterfaces::EqualizerControllerListenerInterface> listener) {
    if (m_equalizerController) {
        m_equalizerController->addListener(listener);
    }
}

void SmartScreenClient::removeEqualizerControllerListener(
    std::shared_ptr<acsdkEqualizerInterfaces::EqualizerControllerListenerInterface> listener) {
    if (m_equalizerController) {
        m_equalizerController->removeListener(listener);
    }
}

void SmartScreenClient::addContextManagerObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerObserverInterface> observer) {
    if (m_contextManager) {
        m_contextManager->addContextManagerObserver(observer);
    }
}

void SmartScreenClient::removeContextManagerObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerObserverInterface> observer) {
    if (m_contextManager) {
        m_contextManager->removeContextManagerObserver(observer);
    }
}

void SmartScreenClient::addSpeakerManagerObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerManagerObserverInterface> observer) {
    m_speakerManager->addSpeakerManagerObserver(observer);
}

void SmartScreenClient::removeSpeakerManagerObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::SpeakerManagerObserverInterface> observer) {
    m_speakerManager->removeSpeakerManagerObserver(observer);
}

std::shared_ptr<avsCommon::sdkInterfaces::SpeakerManagerInterface> SmartScreenClient::getSpeakerManager() {
    return m_speakerManager;
}

void SmartScreenClient::addSpeechSynthesizerObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::SpeechSynthesizerObserverInterface> observer) {
    m_speechSynthesizer->addObserver(observer);
}

void SmartScreenClient::removeSpeechSynthesizerObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::SpeechSynthesizerObserverInterface> observer) {
    m_speechSynthesizer->removeObserver(observer);
}

void SmartScreenClient::addFocusManagersObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerObserverInterface> observer) {
    m_audioFocusManager->addObserver(observer);
    m_visualFocusManager->addObserver(observer);
}

void SmartScreenClient::removeFocusManagersObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerObserverInterface> observer) {
    m_audioFocusManager->removeObserver(observer);
    m_visualFocusManager->removeObserver(observer);
}

bool SmartScreenClient::setFirmwareVersion(avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion) {
    {
        std::lock_guard<std::mutex> lock(m_softwareInfoSenderMutex);

        if (!m_softwareInfoSender) {
            m_softwareInfoSender = capabilityAgents::system::SoftwareInfoSender::create(
                firmwareVersion,
                true,
                m_softwareInfoSenderObservers,
                m_connectionManager,
                m_connectionManager,
                m_exceptionSender);
            if (m_softwareInfoSender) {
                return true;
            }

            ACSDK_ERROR(LX("setFirmwareVersionFailed").d("reason", "unableToCreateSoftwareInfoSender"));
            return false;
        }
    }

    return m_softwareInfoSender->setFirmwareVersion(firmwareVersion);
}

std::future<bool> SmartScreenClient::notifyOfWakeWord(
    capabilityAgents::aip::AudioProvider wakeWordAudioProvider,
    avsCommon::avs::AudioInputStream::Index beginIndex,
    avsCommon::avs::AudioInputStream::Index endIndex,
    std::string keyword,
    std::chrono::steady_clock::time_point startOfSpeechTimestamp,
    std::shared_ptr<const std::vector<char>> KWDMetadata) {
    ACSDK_DEBUG5(LX(__func__).d("keyword", keyword).d("connected", m_connectionManager->isConnected()));

    if (!m_connectionManager->isConnected()) {
        std::promise<bool> ret;
        if (capabilityAgents::aip::AudioInputProcessor::KEYWORD_TEXT_STOP == keyword) {
            // Alexa Stop uttered while offline
            ACSDK_INFO(LX("notifyOfWakeWord").d("action", "localStop").d("reason", "stopUtteredWhileNotConnected"));
            stopForegroundActivity();

            // Returning as interaction handled
            ret.set_value(true);
            return ret.get_future();
        } else {
            // Ignore Alexa wake word while disconnected
            ACSDK_INFO(LX("notifyOfWakeWord").d("action", "ignoreAlexaWakeWord").d("reason", "networkDisconnected"));

            // Returning as interaction not handled
            ret.set_value(false);
            return ret.get_future();
        }
    }

    return m_audioInputProcessor->recognize(
        wakeWordAudioProvider,
        capabilityAgents::aip::Initiator::WAKEWORD,
        startOfSpeechTimestamp,
        beginIndex,
        endIndex,
        keyword,
        KWDMetadata);
}

std::future<bool> SmartScreenClient::notifyOfTapToTalk(
    capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
    avsCommon::avs::AudioInputStream::Index beginIndex,
    std::chrono::steady_clock::time_point startOfSpeechTimestamp) {
    ACSDK_DEBUG5(LX(__func__));
    return m_audioInputProcessor->recognize(
        tapToTalkAudioProvider, capabilityAgents::aip::Initiator::TAP, startOfSpeechTimestamp, beginIndex);
}

std::future<bool> SmartScreenClient::notifyOfHoldToTalkStart(
    capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
    std::chrono::steady_clock::time_point startOfSpeechTimestamp,
    avsCommon::avs::AudioInputStream::Index beginIndex) {
    ACSDK_DEBUG5(LX(__func__));
    return m_audioInputProcessor->recognize(
        holdToTalkAudioProvider, capabilityAgents::aip::Initiator::PRESS_AND_HOLD, startOfSpeechTimestamp, beginIndex);
}

std::future<bool> SmartScreenClient::notifyOfHoldToTalkEnd() {
    return m_audioInputProcessor->stopCapture();
}

std::future<bool> SmartScreenClient::notifyOfTapToTalkEnd() {
    return m_audioInputProcessor->stopCapture();
}

void SmartScreenClient::addAudioInputProcessorObserver(
    const std::shared_ptr<avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface>& observer) {
    m_audioInputProcessor->addObserver(observer);
}

void SmartScreenClient::removeAudioInputProcessorObserver(
    const std::shared_ptr<avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface>& observer) {
    m_audioInputProcessor->removeObserver(observer);
}

void SmartScreenClient::addCallStateObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::CallStateObserverInterface> observer) {
    if (m_callManager) {
        m_callManager->addObserver(observer);
    }
}

void SmartScreenClient::removeCallStateObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::CallStateObserverInterface> observer) {
    if (m_callManager) {
        m_callManager->removeObserver(observer);
    }
}

std::shared_ptr<EndpointBuilderInterface> SmartScreenClient::createEndpointBuilder() {
    return alexaClientSDK::endpoints::EndpointBuilder::create(
        m_deviceInfo, m_contextManager, m_exceptionSender, m_alexaMessageSender);
}

std::shared_ptr<EndpointBuilderInterface> SmartScreenClient::getDefaultEndpointBuilder() {
    return m_defaultEndpointBuilder;
}

std::future<alexaClientSDK::endpoints::EndpointRegistrationManager::RegistrationResult> SmartScreenClient::
    registerEndpoint(std::shared_ptr<EndpointInterface> endpoint) {
    if (m_endpointRegistrationManager) {
        return m_endpointRegistrationManager->registerEndpoint(endpoint);
    } else {
        ACSDK_ERROR(LX("registerEndpointFailed").d("reason", "invalid EndpointRegistrationManager"));
        std::promise<alexaClientSDK::endpoints::EndpointRegistrationManager::RegistrationResult> promise;
        promise.set_value(alexaClientSDK::endpoints::EndpointRegistrationManager::RegistrationResult::INTERNAL_ERROR);
        return promise.get_future();
    }
}

std::future<alexaClientSDK::endpoints::EndpointRegistrationManager::DeregistrationResult> SmartScreenClient::
    deregisterEndpoint(EndpointIdentifier endpointId) {
    if (m_endpointRegistrationManager) {
        return m_endpointRegistrationManager->deregisterEndpoint(endpointId);
    } else {
        ACSDK_ERROR(LX("deregisterEndpointFailed").d("reason", "invalid EndpointRegistrationManager"));
        std::promise<alexaClientSDK::endpoints::EndpointRegistrationManager::DeregistrationResult> promise;
        promise.set_value(alexaClientSDK::endpoints::EndpointRegistrationManager::DeregistrationResult::INTERNAL_ERROR);
        return promise.get_future();
    }
}

bool SmartScreenClient::isCommsEnabled() {
    return (m_callManager != nullptr);
}

void SmartScreenClient::acceptCommsCall() {
    if (m_callManager) {
        m_callManager->acceptCall();
    }
}

void SmartScreenClient::sendDtmf(avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone dtmfTone) {
    if (m_callManager) {
        m_callManager->sendDtmf(dtmfTone);
    }
}

void SmartScreenClient::stopCommsCall() {
    if (m_callManager) {
        m_callManager->stopCall();
    }
}

void SmartScreenClient::addAlexaPresentationObserver(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::AlexaPresentationObserverInterface> observer) {
    if (!m_alexaPresentation) {
        ACSDK_ERROR(LX("addAlexaPresentationObserverFailed").d("reason", "guiNotSupported"));
        return;
    }
    m_alexaPresentation->addObserver(observer);
}

void SmartScreenClient::removeAlexaPresentationObserver(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::AlexaPresentationObserverInterface> observer) {
    if (!m_alexaPresentation) {
        ACSDK_ERROR(LX("removeAlexaPresentationObserverFailed").d("reason", "guiNotSupported"));
        return;
    }
    m_alexaPresentation->removeObserver(observer);
}

void SmartScreenClient::sendUserEvent(const std::string& payload) {
    m_alexaPresentation->sendUserEvent(payload);
}

void SmartScreenClient::sendDataSourceFetchRequestEvent(const std::string& type, const std::string& payload) {
    m_alexaPresentation->sendDataSourceFetchRequestEvent(type, payload);
}

void SmartScreenClient::sendRuntimeErrorEvent(const std::string& payload) {
    m_alexaPresentation->sendRuntimeErrorEvent(payload);
}

void SmartScreenClient::handleVisualContext(uint64_t token, std::string payload) {
    m_alexaPresentation->onVisualContextAvailable(token, payload);
}

void SmartScreenClient::handleRenderDocumentResult(std::string token, bool result, std::string error) {
    m_alexaPresentation->processRenderDocumentResult(token, result, error);
}

void SmartScreenClient::handleExecuteCommandsResult(std::string token, bool result, std::string error) {
    m_alexaPresentation->processExecuteCommandsResult(token, result, error);
}

void SmartScreenClient::handleActivityEvent(
    const std::string& source,
    alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent event,
    bool isAlexaPresentationPresenting) {
    if (isAlexaPresentationPresenting) {
        m_alexaPresentation->processActivityEvent(source, event);
    } else {
        m_templateRuntime->processActivityEvent(source, event);
    }
}

void SmartScreenClient::setDocumentIdleTimeout(std::chrono::milliseconds timeout) {
    m_alexaPresentation->setDocumentIdleTimeout(timeout);
}

void SmartScreenClient::clearAllExecuteCommands() {
    m_alexaPresentation->clearAllExecuteCommands();
}

void SmartScreenClient::setDeviceWindowState(const std::string& payload) {
    m_visualCharacteristics->setDeviceWindowState(payload);
}

std::chrono::milliseconds SmartScreenClient::getDeviceTimezoneOffset() {
    return m_deviceTimeZoneOffset;
}

void SmartScreenClient::handleRenderComplete(bool isAlexaPresentationPresenting) {
    if (isAlexaPresentationPresenting) {
        m_alexaPresentation->recordRenderComplete();
    }
}

void SmartScreenClient::handleDropFrameCount(uint64_t dropFrameCount, bool isAlexaPresentationPresenting) {
    if (isAlexaPresentationPresenting) {
        m_alexaPresentation->recordDropFrameCount(dropFrameCount);
    }
}

void SmartScreenClient::handleAPLEvent(APLClient::AplRenderingEvent event, bool isAlexaPresentationPresenting) {
    if (isAlexaPresentationPresenting) {
        m_alexaPresentation->recordAPLEvent(event);
    }
}

void SmartScreenClient::audioPlayerLocalStop() {
    if (m_audioPlayer) {
        m_audioPlayer->stopPlayback();
    }
}

bool SmartScreenClient::isCommsCallMuted() {
    if (m_callManager) {
        return m_callManager->isSelfMuted();
    }
    return false;
}

void SmartScreenClient::muteCommsCall() {
    if (m_callManager) {
        m_callManager->muteSelf();
    }
}

void SmartScreenClient::unmuteCommsCall() {
    if (m_callManager) {
        m_callManager->unmuteSelf();
    }
}

void SmartScreenClient::onSystemClockSynchronized() {
    m_systemClockMonitor->notifySystemClockSynchronized();
}

void SmartScreenClient::registerExternalMediaPlayerAdapterHandler(
    std::shared_ptr<alexaClientSDK::acsdkExternalMediaPlayerInterfaces::ExternalMediaAdapterHandlerInterface>
        externalMediaPlayerAdapterHandler) {
    if (m_externalMediaPlayer) {
        m_externalMediaPlayer->addAdapterHandler(externalMediaPlayerAdapterHandler);
    }
}

std::shared_ptr<acsdkShutdownManagerInterfaces::ShutdownManagerInterface> SmartScreenClient::getShutdownManager() {
    return m_shutdownManager;
}

SmartScreenClient::~SmartScreenClient() {
    while (!m_shutdownObjects.empty()) {
        if (m_shutdownObjects.back()) {
            m_shutdownObjects.back()->shutdown();
        }
        m_shutdownObjects.pop_back();
    }

    if (m_directiveSequencer) {
        ACSDK_DEBUG5(LX("DirectiveSequencerShutdown"));
        m_directiveSequencer->shutdown();
    }
    if (m_alexaPresentation) {
        ACSDK_DEBUG5(LX("AlexaPresentationShutdown"));
        m_alexaPresentation->shutdown();
    }
    if (m_templateRuntime) {
        ACSDK_DEBUG5(LX("TemplateRuntimeShutdown"));
        m_templateRuntime->shutdown();
    }
    if (m_audioInputProcessor) {
        ACSDK_DEBUG5(LX("AIPShutdown"));
        removeInternetConnectionObserver(m_audioInputProcessor);
        m_audioInputProcessor->shutdown();
    }
    if (m_speechSynthesizer) {
        ACSDK_DEBUG5(LX("SpeechSynthesizerShutdown"));
        m_speechSynthesizer->shutdown();
    }
    if (m_alertsCapabilityAgent) {
        m_systemClockMonitor->removeSystemClockMonitorObserver(m_alertsCapabilityAgent);
        ACSDK_DEBUG5(LX("AlertsShutdown"));
        m_alertsCapabilityAgent->shutdown();
    }
    if (m_softwareInfoSender) {
        ACSDK_DEBUG5(LX("SoftwareInfoShutdown"));
        m_softwareInfoSender->shutdown();
    }
    if (m_messageRouter) {
        ACSDK_DEBUG5(LX("MessageRouterShutdown."));
        m_messageRouter->shutdown();
    }
    if (m_certifiedSender) {
        ACSDK_DEBUG5(LX("CertifiedSenderShutdown."));
        m_certifiedSender->shutdown();
    }
    if (m_visualActivityTracker) {
        ACSDK_DEBUG5(LX("VisualActivityTrackerShutdown."));
        m_visualActivityTracker->shutdown();
    }
    if (m_notificationsCapabilityAgent) {
        ACSDK_DEBUG5(LX("NotificationsShutdown."));
        m_notificationsCapabilityAgent->shutdown();
    }
    if (m_notificationsRenderer) {
        ACSDK_DEBUG5(LX("NotificationsRendererShutdown."));
        m_notificationsRenderer->shutdown();
    }
    if (m_bluetooth) {
        ACSDK_DEBUG5(LX("BluetoothShutdown."));
        m_bluetooth->shutdown();
    }

    if (m_userInactivityMonitor) {
        ACSDK_DEBUG5(LX("UserInactivityMonitorShutdown."));
        m_userInactivityMonitor->shutdown();
    }

    if (m_apiGatewayCapabilityAgent) {
        ACSDK_DEBUG5(LX("CallApiGatewayCapabilityAgentShutdown."));
        m_apiGatewayCapabilityAgent->shutdown();
    }

#ifdef ENABLE_PCC
    if (m_phoneCallControllerCapabilityAgent) {
        ACSDK_DEBUG5(LX("PhoneCallControllerCapabilityAgentShutdown"));
        m_phoneCallControllerCapabilityAgent->shutdown();
    }
#endif
#ifdef ENABLE_MCC
    if (m_meetingClientControllerCapabilityAgent) {
        ACSDK_DEBUG5(LX("MeetingClientControllerCapabilityAgentShutdown"));
        m_meetingClientControllerCapabilityAgent->shutdown();
    }
#endif
    if (m_dndCapabilityAgent) {
        ACSDK_DEBUG5(LX("DNDCapabilityAgentShutdown"));
        removeConnectionObserver(m_dndCapabilityAgent);
        m_dndCapabilityAgent->shutdown();
    }

    if (m_visualCharacteristics) {
        m_visualCharacteristics->shutdown();
    }

    if (nullptr != m_equalizerCapabilityAgent) {
        for (auto& equalizer : m_equalizerRuntimeSetup->getAllEqualizers()) {
            m_equalizerController->unregisterEqualizer(equalizer);
        }
        for (auto& listener : m_equalizerRuntimeSetup->getAllEqualizerControllerListeners()) {
            m_equalizerController->removeListener(listener);
        }
        ACSDK_DEBUG5(LX("EqualizerCapabilityAgentShutdown"));
        m_equalizerCapabilityAgent->shutdown();
    }

    if (m_deviceSettingStorage) {
        ACSDK_DEBUG5(LX("CloseSettingStorage"));
        m_deviceSettingStorage->close();
    }

    if (m_diagnostics) {
        m_diagnostics->setDiagnosticDependencies(nullptr, nullptr, nullptr);

        auto deviceProperties = m_diagnostics->getDevicePropertyAggregator();
        if (deviceProperties) {
            deviceProperties->setContextManager(nullptr);
            deviceProperties->setDeviceSettingsManager(nullptr);
            removeSpeakerManagerObserver(deviceProperties);
            removeAlertsObserver(deviceProperties);
            removeConnectionObserver(deviceProperties);
            removeNotificationsObserver(deviceProperties);
            removeAudioPlayerObserver(deviceProperties);
            removeAlexaDialogStateObserver(deviceProperties);
            m_authDelegate->removeAuthObserver(deviceProperties);
        }

        auto protocolTrace = m_diagnostics->getProtocolTracer();
        if (protocolTrace) {
            removeMessageObserver(protocolTrace);
        }
    }
}

std::chrono::milliseconds SmartScreenClient::calculateDeviceTimezoneOffset(const std::string& timeZone) {
#ifdef _MSC_VER
    TIME_ZONE_INFORMATION TimeZoneInfo;
    GetTimeZoneInformation(&TimeZoneInfo);
    auto offsetInMinutes = -TimeZoneInfo.Bias - TimeZoneInfo.DaylightBias;
    ACSDK_DEBUG9(LX(__func__).m(std::to_string(offsetInMinutes)));
    return std::chrono::minutes(offsetInMinutes);
#else
    char* prevTZ = getenv("TZ");
    setenv("TZ", timeZone.c_str(), 1);
    time_t t = time(NULL);
    struct tm* structtm = localtime(&t);
    if (prevTZ) {
        setenv("TZ", prevTZ, 1);
    } else {
        unsetenv("TZ");
    }
    return std::chrono::milliseconds(structtm->tm_gmtoff * 1000);
#endif
}

void SmartScreenClient::releaseAllObserversOnDialogChannel() {
    for (const auto& observer : m_dialogChannelObserverInterfaces) {
        m_audioFocusManager->releaseChannel(alexaClientSDK::afml::FocusManager::DIALOG_CHANNEL_NAME, observer);
    }
}

void SmartScreenClient::addDialogChannelObserverInterface(const std::shared_ptr<ChannelObserverInterface> &observer) {
    m_dialogChannelObserverInterfaces.insert(observer);
}

void SmartScreenClient::removeDialogChannelObserverInterface(const std::shared_ptr<ChannelObserverInterface> &observer) {
    m_dialogChannelObserverInterfaces.erase(observer);
}

}  // namespace smartScreenClient
}  // namespace alexaSmartScreenSDK
