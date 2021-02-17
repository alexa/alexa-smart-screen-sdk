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

#ifndef APLCLIENTSANDBOX_INCLUDE_APLCLIENTBINDING_H_
#define APLCLIENTSANDBOX_INCLUDE_APLCLIENTBINDING_H_

#include <APLClient/AplClientBinding.h>
#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include <APLClient/Extensions/AudioPlayer/AplAudioPlayerExtension.h>
#include <APLClient/Extensions/AudioPlayer/AplAudioPlayerAlarmsExtension.h>
#include <APLClient/Extensions/Backstack/AplBackstackExtension.h>
#include <APLClient/AplOptionsInterface.h>
#include <mutex>
#include <string>
#include "APLClientSandbox/Executor.h"
#include "GUIManager.h"


class GUIManager;

class AplClientBridge
        : public APLClient::AplOptionsInterface
        , public APLClient::Extensions::Backstack::AplBackstackExtensionObserverInterface
        , public APLClient::Extensions::AudioPlayer::AplAudioPlayerExtensionObserverInterface
        , public APLClient::Extensions::AudioPlayer::AplAudioPlayerAlarmsExtensionObserverInterface
        , public std::enable_shared_from_this<AplClientBridge> {
public:
    static std::shared_ptr<AplClientBridge> create();

    virtual ~AplClientBridge() = default;

    /**
     * Perform an update loop of the APL Core Engine
     */
    void updateTick();

    /**
     * Renders the given document
     * @param document
     * @param data
     * @param supportedViewports
     */
    void renderDocument(const std::string& document, const std::string& data, const std::string& supportedViewports);

    /**
     * Clears the current document
     */
    void clearDocument();

    /**
     * Executes the given command payload
     * @param jsonPayload The command
     */
    void executeCommands(const std::string& jsonPayload);

    /**
     * Interrupts the currently executing command sequence
     */
    void interruptCommandSequence();

    /**
     * Should be called when a message is received from the viewhost
     * @param message the viewhost message
     */
    void onMessage(const std::string& message);

    /// @name AplBackstackExtensionObserverInterface Functions
    /// @{
    void onRestoreDocumentState(std::shared_ptr<APLClient::AplDocumentState> documentState) override;
    /// @}

    /// @name AplAudioPlayerExtensionObserverInterface Functions
    /// @{
    void onAudioPlayerPlay() override;
    void onAudioPlayerPause() override;
    void onAudioPlayerNext() override;
    void onAudioPlayerPrevious() override;
    void onAudioPlayerSeekToPosition(int offsetInMilliseconds) override;
    void onAudioPlayerToggle(const std::string& name, bool checked) override;
    void onAudioPlayerLyricDataFlushed(const std::string& token,
                                       long durationInMilliseconds,
                                       const std::string& lyricData) override;
    void onAudioPlayerSkipForward() override;
    void onAudioPlayerSkipBackward() override;
    /// @}

    /// @name AplAudioPlayerAlarmsExtensionObserverInterface Functions
    /// @{
    void onAudioPlayerAlarmDismiss() override;
    void onAudioPlayerAlarmSnooze() override;
    /// @}

    bool handleBack();

    /**
     * Loads default extensions managed by the @c AplClientBridge
     */
    void loadExtensions();

    /**
     *  Adds @c AplCoreExtensionInterface extensions to be registered with the @c AplClient
     * @param extensions Set of pointers to @c AplCoreExtensionInterface extensions.
     */
    void addExtensions(
        std::unordered_set<std::shared_ptr<APLClient::Extensions::AplCoreExtensionInterface>> extensions);

    /**
     * Sets the GUI Manager
     * @param manager
     */
    void setGUIManager(std::shared_ptr<GUIManager> manager);

    /**
     * To be called when a resource has been retrieved
     * @param url The URL of the retrieved resource
     * @param payload The payload of the resource
     */
    void provideResource(const std::string& url, const std::string& payload);

    /// @name AplOptionsInterface functions
    /// @{
    void sendMessage(const std::string& token, const std::string& payload) override;
    void resetViewhost(const std::string& token) override;
    std::string downloadResource(const std::string& source) override;
    std::chrono::milliseconds getTimezoneOffset() override;
    void onActivityStarted(const std::string& token, const std::string& source) override;
    void onActivityEnded(const std::string& token, const std::string& source) override;
    void onSendEvent(const std::string& token, const std::string& event) override;
    void onCommandExecutionComplete(const std::string& token, bool result) override;
    void onRenderDocumentComplete(const std::string& token, bool result, const std::string& error) override;
    void onVisualContextAvailable(
            const std::string& token,
            unsigned int stateRequestToken,
            const std::string& context) override;
    void onSetDocumentIdleTimeout(const std::string& token, const std::chrono::milliseconds& timeout) override;
    void onRenderingEvent(const std::string& token, APLClient::AplRenderingEvent event) override;
    void onFinish(const std::string& token) override;
    void onDataSourceFetchRequestEvent(
            const std::string& token,
            const std::string& type,
            const std::string& payload) override;
    void onRuntimeErrorEvent(const std::string& token, const std::string& payload) override;
    void logMessage(APLClient::LogLevel level, const std::string& source, const std::string& message) override;
    int getMaxNumberOfConcurrentDownloads() override;
    void onExtensionEvent(
        const std::string& token,
        const std::string& uri,
        const std::string& name,
        const std::string& source,
        const std::string& params,
        unsigned int event,
        std::shared_ptr<APLClient::Extensions::AplCoreExtensionEventCallbackResultInterface> resultCallback) override;
    /// @}

    /**
     * Retrieves the current time
     * @return The time
     */
    std::chrono::milliseconds getCurrentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
    }

private:
    /// Private constructor
    AplClientBridge();

    /// The GUI Manager
    std::weak_ptr<GUIManager> m_manager;

    /// The APL Client Binding
    std::shared_ptr<APLClient::AplClientBinding> m_client;

    /// Pointer to the APL Client Renderer
    std::shared_ptr<APLClient::AplClientRenderer> m_aplClientRenderer;

    /// Mutex to prevent more than one resource being downloaded simultaneously
    std::mutex m_downloadMutex;

    /// Promise set when a resoure request it made, should be resolved once it has been retrieved
    std::promise<std::string> m_resourcePromise;

    /// The url from the outstanding resource request
    std::string m_resourcePromiseUrl;

    /// The execution thread
    Executor m_executor;

    /// Pointer to the @c AplBackstackExtension
    std::shared_ptr<APLClient::Extensions::Backstack::AplBackstackExtension> m_backstackExtension;

    /// Pointer to the @c AplAudioPlayerExtension
    std::shared_ptr<APLClient::Extensions::AudioPlayer::AplAudioPlayerExtension> m_audioPlayerExtension;

    /// Pointer to the @c AplAudioPlayerAlarmsExtension
    std::shared_ptr<APLClient::Extensions::AudioPlayer::AplAudioPlayerAlarmsExtension> m_audioPlayerAlarmsExtension;

    /// audioPlayer offset at current session
    int audioPlayerOffset;

    /// audioPlayer initialize time
    int audioPlayerStartTime;

    /// audioPlayer is playing or not.
    bool audioPlayerPlaying = false;
};

#endif  // APLCLIENTSANDBOX_INCLUDE_APLCLIENTBINDING_H_
