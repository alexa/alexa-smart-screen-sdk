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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_GUICLIENTMESSAGE_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_GUICLIENTMESSAGE_H

#include <string>

#include <rapidjson/document.h>

#include <AVSCommon/AVS/FocusState.h>
#include <AVSCommon/SDKInterfaces/CallStateObserverInterface.h>
#include <SmartScreenSDKInterfaces/AudioPlayerInfo.h>
#include <Captions/CaptionFrame.h>

#include "Message.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace messages {

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

/// The message type for renderCaptions.
const std::string GUI_MSG_TYPE_RENDER_CAPTIONS("renderCaptions");

/// The message type for localeChange.
const std::string GUI_MSG_TYPE_LOCALE_CHANGE("localeChange");

/// The message type for DoNotDisturbStateChanged.
const std::string GUI_MSG_TYPE_DND_SETTING_CHANGE("doNotDisturbSettingChanged");

/// The doNotDisturbEnabled json key in the message.
const std::string GUI_MSG_TYPE_DND_SETTING_TAG("doNotDisturbSettingEnabled");

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

/// The message type for callStateChange.
const std::string GUI_MSG_TYPE_CALL_STATE_CHANGE("callStateChange");

/// The callState json key in the message.
const std::string GUI_MSG_CALL_STATE_TAG("callState");

/// The callType json key in the message.
const std::string GUI_MSG_CALL_TYPE_TAG("callType");

/// The previousSipUserAgentState json key in the message.
const std::string GUI_MSG_PREVIOUS_SIP_USER_AGENT_STATE_TAG("previousSipUserAgentState");

/// The currentSipUserAgentState json key in the message.
const std::string GUI_MSG_CURRENT_SIP_USER_AGENT_STATE_TAG("currentSipUserAgentState");

/// The displayName json key in the message.
const std::string GUI_MSG_DISPLAY_NAME_TAG("displayName");

/// The endpointLabel json key in the message.
const std::string GUI_MSG_END_POINT_LABEL_TAG("endpointLabel");

/// The inboundCalleeName json key in the message.
const std::string GUI_MSG_INBOUND_CALLEE_NAME_TAG("inboundCalleeName");

/// The callProviderType json key in the message.
const std::string GUI_MSG_CALL_PROVIDER_TYPE_TAG("callProviderType");

/// The inboundRingtoneUrl json key in the message.
const std::string GUI_MSG_INBOUND_RINGTONE_URL_TAG("inboundRingtoneUrl");

/// The outboundRingbackUrl json key in the message.
const std::string GUI_MSG_OUTBOUND_RINGBACK_URL_TAG("outboundRingbackUrl");

/// The isDropIn json key in the message.
const std::string GUI_MSG_IS_DROP_IN_TAG("isDropIn");

/// The locales json key in the message.
const std::string GUI_MSG_LOCALES_TAG("locales");

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
    GUIClientMessage(const std::string& type) : Message(type), mParsedDocument(&alloc()) {
    }

    /**
     * Sets the json payload for this message
     * @param payload The payload to parse and send
     * @param tag The optional tag for the json payload.  Default is 'payload'.
     * @return this
     */
    GUIClientMessage& setParsedPayload(const std::string& payload, const std::string& tag = MSG_PAYLOAD_TAG) {
        mDocument.AddMember(
            rapidjson::Value(tag.c_str(), mDocument.GetAllocator()).Move(),
            mParsedDocument.Parse(payload),
            mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the windowId for this message
     * @param windowId The target windowId of
     * @return this
     */
    GUIClientMessage& setWindowId(const std::string& windowId) {
        mDocument.AddMember(MSG_WINDOWID_TAG, windowId, mDocument.GetAllocator());
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

#ifdef ENABLE_COMMS
/**
 * The @c CallStateChangeMessage contains information for communicating call state info to the GUI Client.
 */
class CallStateChangeMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param callStateInfo The Comms client call state info.
     */
    CallStateChangeMessage(
        const alexaClientSDK::avsCommon::sdkInterfaces::CallStateObserverInterface::CallStateInfo& callStateInfo) :
            GUIClientMessage(GUI_MSG_TYPE_CALL_STATE_CHANGE) {
        std::ostringstream oss;
        oss << callStateInfo.callState;
        addMember(GUI_MSG_CALL_STATE_TAG, oss.str());
        addMember(GUI_MSG_CALL_TYPE_TAG, callStateInfo.callType);
        addMember(GUI_MSG_PREVIOUS_SIP_USER_AGENT_STATE_TAG, callStateInfo.previousSipUserAgentState);
        addMember(GUI_MSG_CURRENT_SIP_USER_AGENT_STATE_TAG, callStateInfo.currentSipUserAgentState);
        addMember(GUI_MSG_DISPLAY_NAME_TAG, callStateInfo.displayName);
        addMember(GUI_MSG_END_POINT_LABEL_TAG, callStateInfo.endpointLabel);
        addMember(GUI_MSG_INBOUND_CALLEE_NAME_TAG, callStateInfo.inboundCalleeName);
        addMember(GUI_MSG_CALL_PROVIDER_TYPE_TAG, callStateInfo.callProviderType);
        addMember(GUI_MSG_INBOUND_RINGTONE_URL_TAG, callStateInfo.inboundRingtoneUrl);
        addMember(GUI_MSG_OUTBOUND_RINGBACK_URL_TAG, callStateInfo.outboundRingbackUrl);
        addMember(GUI_MSG_IS_DROP_IN_TAG, callStateInfo.isDropIn);
    }
};
#endif

/**
 * The @c GuiConfigurationMessage contains configuration data for configuring the windows and functionality of the GUI
 * Client.
 */
class GuiConfigurationMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param visualCharacteristics The visualCharacteristics config string.
     * @param appConfig The appConfig config string.
     */
    GuiConfigurationMessage(std::string visualCharacteristics, std::string appConfig) :
            GUIClientMessage(GUI_MSG_TYPE_GUI_CONFIG) {
        rapidjson::Value payload(rapidjson::kObjectType);
        rapidjson::Document messageDocument(&alloc());
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
    FocusChangedMessage(unsigned token, alexaClientSDK::avsCommon::avs::FocusState focusState) :
            GUIClientMessage(GUI_MSG_TYPE_ON_FOCUS_CHANGED) {
        setToken(token);
        addMember(GUI_MSG_CHANNEL_STATE_TAG, focusStateToString(focusState));
    }
};

/**
 * The @c FocusResponseMessage provides the GUI Client with the result of `focusAcquireRequest` and
 * `focusReleaseRequest` requests processing.
 */
class FocusResponseMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param token The requestor token.
     * @param result The result of focus request processing.
     */
    FocusResponseMessage(unsigned token, bool result) : GUIClientMessage(GUI_MSG_TYPE_FOCUS_RESPONSE) {
        setToken(token);
        addMember(GUI_MSG_RESULT_TAG, result ? "true" : "false");
    }
};

/**
 * The @c AuthorizationRequestMessage provides the GUI Client with information to present to the user to complete CBL
 * device authorization.
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
    AuthorizationRequestMessage(std::string url, std::string code, std::string clientId) :
            GUIClientMessage(GUI_MSG_TYPE_REQUEST_AUTH) {
        addMember(GUI_MSG_AUTH_URL_TAG, url);
        addMember(GUI_MSG_AUTH_CODE_TAG, code);
        addMember(GUI_MSG_CLIENT_ID_TAG, clientId);
    }
};

/**
 * The @c AuthorizationChangedMessage provides the GUI Client with information about changes to the state of
 * authorization.
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
 * The @c AplRenderMessage provides the GUI Client with information to trigger an APL document render in the targeted
 * window.
 */
class AplRenderMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param windowId The id of the window to target with the APL document render.
     * @param token The presentation token of the APL document to render
     */
    AplRenderMessage(std::string windowId, std::string token) : GUIClientMessage(GUI_MSG_TYPE_APL_RENDER) {
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
     * @param windowId The target windowId for this message.
     * @param payload The APL Core message object to serialize.
     */
    AplCoreMessage(std::string windowId, std::string payload) : GUIClientMessage(GUI_MSG_TYPE_APL_CORE) {
        setWindowId(windowId);
        setParsedPayload(payload);
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
 *  The @c RenderPlayerInfoMessage instructs the GUI Client to display visual metadata associated with a media item,
 * such as a song or playlist. It contains the datasource and AudioPlayer state information required to synchronize the
 * UI with the active AudioPlayer.
 */
class RenderPlayerInfoMessage : public GUIClientMessage {
public:
    /**
     * Constructor.
     *
     * @param jsonPayload The RenderPlayerInfo payload.
     * @param audioPlayerInfo @c The smartScreenSDKInterfaces::AudioPlayerInfo object containing player state and offet
     * values.
     */
    RenderPlayerInfoMessage(std::string jsonPayload, smartScreenSDKInterfaces::AudioPlayerInfo audioPlayerInfo) :
            GUIClientMessage(GUI_MSG_TYPE_RENDER_PLAYER_INFO) {
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
    ClearRenderTemplateCardMessage(std::string windowId) : GUIClientMessage(GUI_MSG_TYPE_CLEAR_TEMPLATE_CARD) {
        setWindowId(windowId);
    }
};

/**
 *  The @c ClearPlayerInfoCardMessage instructs the GUI Client to clear the audio media player UI from the screen.
 */
class ClearPlayerInfoCardMessage : public GUIClientMessage {
public:
    ClearPlayerInfoCardMessage() : GUIClientMessage(GUI_MSG_TYPE_CLEAR_PLAYER_INFO_CARD) {
    }
};

/**
 *  The @c ClearDocumentMessage instructs the GUI Client to clear visual content from the screen.
 */
class ClearDocumentMessage : public GUIClientMessage {
public:
    ClearDocumentMessage(std::string windowId) : GUIClientMessage(GUI_MSG_TYPE_CLEAR_DOCUMENT) {
        setWindowId(windowId);
    }
};

/**
 *  The @c RenderCaptionsMessage instructs the GUI Client to render captions
 */
class RenderCaptionsMessage : public GUIClientMessage {
public:
    explicit RenderCaptionsMessage(const std::string& payload) : GUIClientMessage(GUI_MSG_TYPE_RENDER_CAPTIONS) {
        setParsedPayload(payload);
    };
};

/**
 *  The @c DoNotDisturbSettingChange instructs the GUI Client to render DoNotDisturb VoiceChrome
 */
class DoNotDisturbSettingChangedMessage : public GUIClientMessage {
public:
    explicit DoNotDisturbSettingChangedMessage(const bool payload) : GUIClientMessage(GUI_MSG_TYPE_DND_SETTING_CHANGE) {
        addMember(GUI_MSG_TYPE_DND_SETTING_TAG, payload);
    }
};

/**
 *  The @c LocaleChangeMessage informs the GUI Client of Alexa locale setting changes.
 */
class LocaleChangeMessage : public GUIClientMessage {
public:
    explicit LocaleChangeMessage(const std::string& payload) : GUIClientMessage(GUI_MSG_TYPE_LOCALE_CHANGE) {
        setParsedPayload(payload, GUI_MSG_LOCALES_TAG);
    }
};

}  // namespace messages
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_GUICLIENTMESSAGE_H
