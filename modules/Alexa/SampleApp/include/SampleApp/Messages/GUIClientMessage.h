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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_GUICLIENTMESSAGE_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_GUICLIENTMESSAGE_H

#include <string>

#include <rapidjson/document.h>

#include <AVSCommon/AVS/FocusState.h>
#include <SmartScreenSDKInterfaces/AudioPlayerInfo.h>

#include "Message.h"

/// The message type for initRequest.
const std::string GUI_MSG_TYPE_INIT_REQUEST("initRequest");

/// The message type for guiConfiguration.
const std::string GUI_MSG_TYPE_GUI_CONFIG("guiConfiguration");

/// The message type for alexaStateChanged.
const std::string GUI_MSG_TYPE_ALEXA_STATE_CHANGED("alexaStateChanged");

/// The message type for onFocusChanged.
const std::string GUI_MSG_TYPE_ON_FOCUS_CHANGED("onFocusChanged");

/// The message type for focusResponse.
const std::string GUI_MSG_TYPE_FOCUS_RESPONSE("focusResponse");

/// The message type for requestAuthorization.
const std::string GUI_MSG_TYPE_REQUEST_AUTH("requestAuthorization");

/// The message type for authorizationChange.
const std::string GUI_MSG_TYPE_AUTH_CHANGED("authorizationChange");

/// The message type for apl render.
const std::string GUI_MSG_TYPE_APL_RENDER("aplRender");

/// The message type for apl core.
const std::string GUI_MSG_TYPE_APL_CORE("aplCore");

/// The message type for renderTemplate.
const std::string GUI_MSG_TYPE_RENDER_TEMPLATE("renderTemplate");

/// The message type for renderPlayerInfo.
const std::string GUI_MSG_TYPE_RENDER_PLAYER_INFO("renderPlayerInfo");

/// The message type for clearTemplateCard.
const std::string GUI_MSG_TYPE_CLEAR_TEMPLATE_CARD("clearTemplateCard");

/// The message type for clearPlayerInfoCard.
const std::string GUI_MSG_TYPE_CLEAR_PLAYER_INFO_CARD("clearPlayerInfoCard");

/// The message type for clearDocument.
const std::string GUI_MSG_TYPE_CLEAR_DOCUMENT("clearDocument");

/// The SSSDK version key in the message.
const std::string GUI_MSG_SMART_SCREEN_SDK_VERSION_TAG("smartScreenSDKVersion");

/// The window json key in the message.
const std::string GUI_MSG_WINDOW_ID_TAG("windowId");

/// The result json key in the message.
const std::string GUI_MSG_RESULT_TAG("result");

/// The channelState json key in the message.
const std::string GUI_MSG_CHANNEL_STATE_TAG("channelState");

/// The auth url json key in the message.
const std::string GUI_MSG_AUTH_URL_TAG("url");

/// The auth code json key in the message.
const std::string GUI_MSG_AUTH_CODE_TAG("code");

/// The clientId json key in the message.
const std::string GUI_MSG_CLIENT_ID_TAG("clientId");

/// The visualCharacteristics json key in the message.
const char GUI_MSG_VISUALCHARACTERISTICS_TAG[] = "visualCharacteristics";

/// The appConfig json key in the message.
const char GUI_MSG_APPCONFIG_TAG[] = "appConfig";

/// The audioPlayerState json key in the message.
const char GUI_MSG_AUDIO_PLAYER_STATE_TAG[] = "audioPlayerState";

/// The audioOffset json key in the message.
const char GUI_MSG_AUDIO_OFFSET_TAG[] = "audioOffset";

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace messages {

/**
 * The @c GUIClientMessage base class for @c Messages sent to GUI Client.
 */
class GUIClientMessage : public Message { 
private:
    rapidjson::Document mParsedDocument;

public:
    /**
     * Constructor
     * @param type The type from this message
     */
    GUIClientMessage(const std::string& type) : Message(type) {}

    /**
     * Sets the json payload for this message
     * @param payload The payload to parse and send
     * @return this
     */
    GUIClientMessage& setParsedPayload(const std::string& payload) {
        mDocument.AddMember(MSG_PAYLOAD_TAG, mParsedDocument.Parse(payload), mDocument.GetAllocator());
        return *this;
    }
};

/**
 * The @c InitRequestMessage contains information for initializing the GUI Client.
 */
class InitRequestMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param smartScreenSDKVersion The version number for the smartScreenSDK.
     */
    InitRequestMessage(std::string smartScreenSDKVersion) : GUIClientMessage(GUI_MSG_TYPE_INIT_REQUEST) {
        addMember(GUI_MSG_SMART_SCREEN_SDK_VERSION_TAG, smartScreenSDKVersion);
    }
};

/**
 * The @c AlexaStateChangedMessage contains information for communicating Alexa state to the GUI Client.
 */
class AlexaStateChangedMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param alexaState The state of the Alexa client.
     */
    AlexaStateChangedMessage(std::string alexaState) : GUIClientMessage(GUI_MSG_TYPE_ALEXA_STATE_CHANGED) {
        setState(alexaState);
    }
};

/**
 * The @c GuiConfigurationMessage contains configuration data for configuring the windows and functionality of the GUI Client.
 */
class GuiConfigurationMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param visualCharacteristics The visualCharacteristics config string.
     * @param appConfig The appConfig config string.
     */
    GuiConfigurationMessage(
        std::string visualCharacteristics,
        std::string appConfig) : GUIClientMessage(GUI_MSG_TYPE_GUI_CONFIG) { 
        
        rapidjson::Value payload(rapidjson::kObjectType);
        rapidjson::Document messageDocument;
        payload.AddMember(GUI_MSG_VISUALCHARACTERISTICS_TAG, messageDocument.Parse(visualCharacteristics), alloc());
        payload.AddMember(GUI_MSG_APPCONFIG_TAG, messageDocument.Parse(appConfig), alloc());
        setPayload(std::move(payload));
    }
};

/**
 * The @c FocusChangedMessage provides the GUI Client with Focus state changes on corresponding channel.
 */
class FocusChangedMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param token The requestor token.
     * @param channelState The channel focus state.
     */
    FocusChangedMessage(
        unsigned token,
        alexaClientSDK::avsCommon::avs::FocusState focusState) : GUIClientMessage(GUI_MSG_TYPE_ON_FOCUS_CHANGED) {
        
        setToken(token);
        addMember(GUI_MSG_CHANNEL_STATE_TAG, focusStateToString(focusState));
    }
};

/**
 * The @c FocusResponseMessage provides the GUI Client with the result of `focusAcquireRequest` and `focusReleaseRequest` requests processing.
 */
class FocusResponseMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param token The requestor token.
     * @param result The result of focus request processing.
     */
    FocusResponseMessage(
        unsigned token,
        bool result) : GUIClientMessage(GUI_MSG_TYPE_FOCUS_RESPONSE) {
        
        setToken(token);
        addMember(GUI_MSG_RESULT_TAG, result ? "true" : "false");
    }
};

/**
 * The @c AuthorizationRequestMessage provides the GUI Client with information to present to the user to complete CBL device authorization.
 */
class AuthorizationRequestMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param url The URL that the user needs to navigate to.
     * @param code The code that the user needs to enter once authorized.
     * @param The device's Client Id.
     */
    AuthorizationRequestMessage(
        std::string url,
        std::string code,
        std::string clientId) : GUIClientMessage(GUI_MSG_TYPE_REQUEST_AUTH) {
        
        addMember(GUI_MSG_AUTH_URL_TAG, url);
        addMember(GUI_MSG_AUTH_CODE_TAG, code);
        addMember(GUI_MSG_CLIENT_ID_TAG, clientId);
    }
};

/**
 * The @c AuthorizationChangedMessage provides the GUI Client with information about changes to the state of authorization.
 */
class AuthorizationChangedMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param authorizationState The state of authorization.
     */
    AuthorizationChangedMessage(std::string authorizationState) : GUIClientMessage(GUI_MSG_TYPE_AUTH_CHANGED) {
        setState(authorizationState);
    }
};

/**
 * The @c AplRenderMessage provides the GUI Client with information to trigger an APL document render in the targeted window.
 */
class AplRenderMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param windowId The id of the window to target with the APL document render.
     * @param token The presentation token of the APL document to render
     */
    AplRenderMessage(
        std::string windowId,
        std::string token) : GUIClientMessage(GUI_MSG_TYPE_APL_RENDER) {
        
        addMember(GUI_MSG_WINDOW_ID_TAG, windowId);
        addMember(MSG_TOKEN_TAG, token);
    }
};

/**
 * The @c AplCoreMessage provides drawing updates to the GUI Client's APL renderer.
 */
class AplCoreMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param payload The APL Core message object to serialize.
     */
    AplCoreMessage(rapidjson::Value payload) : GUIClientMessage(GUI_MSG_TYPE_APL_CORE) {
        setPayload(std::move(payload));
    }
};

/**
 * The @c RenderTemplateMessage instructs the GUI Client to draw visual metadata to the screen.
 */
class RenderTemplateMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param jsonPayload The RenderTemplate payload.
     */
    RenderTemplateMessage(std::string jsonPayload) : GUIClientMessage(GUI_MSG_TYPE_RENDER_TEMPLATE) {
        setParsedPayload(jsonPayload);
    }
};

/**
 *  The @c RenderPlayerInfoMessage instructs the GUI Client to display visual metadata associated with a media item, such as a song or playlist. 
 *  It contains the datasource and AudioPlayer state information required to synchronize the UI with the active AudioPlayer.
 */ 
class RenderPlayerInfoMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param jsonPayload The RenderPlayerInfo payload.
     * @param audioPlayerInfo @c The smartScreenSDKInterfaces::AudioPlayerInfo object containing player state and offet values.
     */
    RenderPlayerInfoMessage(
        std::string jsonPayload,
        smartScreenSDKInterfaces::AudioPlayerInfo audioPlayerInfo) : GUIClientMessage(GUI_MSG_TYPE_RENDER_PLAYER_INFO) {

        addMember(GUI_MSG_AUDIO_PLAYER_STATE_TAG, playerActivityToString(audioPlayerInfo.audioPlayerState));
        addMember(GUI_MSG_AUDIO_OFFSET_TAG, audioPlayerInfo.offset.count());
        setParsedPayload(jsonPayload);
    }
};

/**
 *  The @c ClearRenderTemplateCardMessage instructs the GUI Client to clear visual content from the screen.
 */ 
class ClearRenderTemplateCardMessage : public GUIClientMessage {
public:
    ClearRenderTemplateCardMessage() : GUIClientMessage(GUI_MSG_TYPE_CLEAR_TEMPLATE_CARD) {}
};

/**
 *  The @c ClearPlayerInfoCardMessage instructs the GUI Client to clear the audio media player UI from the screen.
 */ 
class ClearPlayerInfoCardMessage : public GUIClientMessage {
public:
    ClearPlayerInfoCardMessage() : GUIClientMessage(GUI_MSG_TYPE_CLEAR_PLAYER_INFO_CARD) {}
};

/**
 *  The @c ClearDocumentMessage instructs the GUI Client to clear visual content from the screen.
 */ 
class ClearDocumentMessage : public GUIClientMessage {
public:
    ClearDocumentMessage() : GUIClientMessage(GUI_MSG_TYPE_CLEAR_DOCUMENT) {}
};
}  // namespace messages
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_GUICLIENTMESSAGE_H
