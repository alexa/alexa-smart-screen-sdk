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

import { ActivityEvent } from '../activity/ActivityEvent';
import { JSLogLevel } from 'apl-client';
import { NavigationEvent } from './NavigationEvent';
import { ContentType } from '../focus/ContentType';

/**
 * Enumerates the different states of the Alexa service.
 *
 * @enum
 * @exports
 */
export enum AlexaState {
    /// Guard for unknown state.
    UNKNOWN = 'UNKNOWN',
    /// Alexa is disconnected.
    DISCONNECTED = 'DISCONNECTED',
    /// Alexa is pending connection.
    CONNECTING = 'CONNECTING',
    /// Alexa connected.
    CONNECTED = 'CONNECTED',
    /// Alexa is idle and ready for an interaction.
    IDLE = 'IDLE',
    /// Alexa is currently listening.
    LISTENING = 'LISTENING',
    /// Alexa is currently expecting a response from the customer.
    EXPECTING = 'EXPECTING',
    /**
     * A customer request has been completed and no more input is accepted. In this state, Alexa is waiting for a
     * response from AVS.
     */
    THINKING = 'THINKING',
    /// Alexa is responding to a request with speech.
    SPEAKING = 'SPEAKING'
}

export enum CallState {
    /// Guard for unknown state.
    UNKNOWN = 'UNKNOWN',
    /// The call is connecting.
    CONNECTING = 'CONNECTING',
    /// An incoming call is causing a ringtone to be played.
    INBOUND_RINGING = 'INBOUND_RINGING',
    /// The call has successfully connected.
    CALL_CONNECTED = 'CALL_CONNECTED',
    /// The call has ended.
    CALL_DISCONNECTED = 'CALL_DISCONNECTED',
    /// No current call state to be relayed to the user.
    NONE = 'NONE'
}

export enum CameraState {
    /// The camera is connecting.
    CONNECTING = 'CONNECTING',
    /// The camera connected.
    CONNECTED = 'CONNECTED',
    /// The camera disconnected.
    DISCONNECTED = 'DISCONNECTED',
    /// The camera encountered an error.
    ERROR = 'ERROR',
    /// Guard for unknown state.
    UNKNOWN = 'UNKNOWN'
}

export type AudioPlayerState =
    'IDLE'
    | 'PLAYING'
    | 'STOPPED'
    | 'PAUSED'
    | 'BUFFER_UNDERRUN'
    | 'FINISHED';

export type AuthorizationState =
    'UNINITIALIZED'
    | 'REFRESHED'
    | 'EXPIRED'
    | 'ERROR'
    | 'AUTHORIZING';

export type LocaleType =
    'de-DE'
    | 'en-AU'
    | 'en-CA'
    | 'en-GB'
    | 'en-IN'
    | 'en-US'
    | 'es-ES'
    | 'es-MX'
    | 'es-US'
    | 'fr-CA'
    | 'fr-FR'
    | 'hi-IN'
    | 'it-IT'
    | 'ja-JP'
    | 'pt-BR'
    | 'ar-SA';

export type InboundMessageType =
    'initRequest'
    | 'alexaStateChanged'
    | 'guiConfiguration'
    | 'requestAuthorization'
    | 'authorizationChange'
    | 'onFocusChanged'
    | 'focusResponse'
    | 'renderTemplate'
    | 'renderPlayerInfo'
    | 'clearTemplateCard'
    | 'clearPlayerInfoCard'
    | 'clearDocument'
    | 'aplRender'
    | 'aplCore'
    | 'renderCaptions'
    | 'doNotDisturbSettingChanged'
    | 'callStateChange'
    | 'renderCamera'
    | 'cameraStateChanged'
    | 'clearCamera'
    | 'dtmfTonesSent'
    | 'videoCallingConfig'
    | 'localeChange';

export interface IBaseInboundMessage {
    type : InboundMessageType;
}

export interface IInitRequest extends IBaseInboundMessage {
    smartScreenSDKVersion : string;
}

export interface IVideoCallingConfigMessage extends IBaseInboundMessage {
    videoCallingConfig : string;
}

export interface ICallStateChangeMessage extends IBaseInboundMessage {
    callState : CallState;
    callType? : string;
    previousSipUserAgentState? : string;
    currentSipUserAgentState? : string;
    displayName? : string;
    endpointLabel? : string;
    inboundCalleeName? : string;
    callProviderType? : string;
    inboundRingtoneUrl? : string;
    outboundRingbackUrl? : string;
    isDropIn? : boolean;
}

export interface IDtmfTonesSentMessage extends IBaseInboundMessage {
    dtmfTones : string;
}

export interface IAlexaStateChangedMessage extends IBaseInboundMessage {
    state : AlexaState;
}

export interface IGuiConfigurationMessage extends IBaseInboundMessage {
    payload : any;
}

export interface IRequestAuthorizationMessage extends IBaseInboundMessage {
    url : string;
    code : string;
    clientId : string;
}

export interface IAuthorizationChangeMessage extends IBaseInboundMessage {
    state : AuthorizationState;
}

export interface IOnFocusChangedMessage extends IBaseInboundMessage {
    token : number;
    focusState : string;
}

export interface IFocusResponseMessage extends IBaseInboundMessage {
    token : number;
    result : boolean;
}

export interface IRenderTemplateMessage extends IBaseInboundMessage {
    token : string;
    payload : any;
}

export interface IRenderPlayerInfoMessage extends IBaseInboundMessage {
    token : string;
    payload : any;
    audioPlayerState : AudioPlayerState;
    audioOffset : number;
}

export interface IAPLRenderMessage extends IBaseInboundMessage {
    windowId? : string;
    token : string;
}

export interface IAPLCoreMessage extends IBaseInboundMessage {
    windowId? : string;
    payload : any;
}

export interface IClearDocumentMessage extends IBaseInboundMessage {
    windowId? : string;
}

export interface IRenderCaptionsMessage extends IBaseInboundMessage {
    payload : any;
}

export interface IRenderCameraMessage extends IBaseInboundMessage {
    payload : any;
    liveViewControllerOptions : any;
}

export interface ICameraStateChangedMessage extends IBaseInboundMessage {
    cameraState : CameraState;
}

export interface IDoNotDisturbSettingChangedMessage extends IBaseInboundMessage {
    doNotDisturbSettingEnabled : boolean;
}

export interface ILocaleChangeMessage extends IBaseInboundMessage {
    locales : LocaleType[];
}

export type OutboundMessageType =
    'initResponse'
    | 'deviceWindowState'
    | 'focusAcquireRequest'
    | 'focusReleaseRequest'
    | 'onFocusChangedReceivedConfirmation'
    | 'tapToTalk'
    | 'holdToTalkStart'
    | 'holdToTalkEnd'
    | 'renderStaticDocument'
    | 'executeCommands'
    | 'aplEvent'
    | 'activityEvent'
    | 'navigationEvent'
    | 'logEvent'
    | 'toggleCaptions'
    | 'toggleDoNotDisturb'
    | 'acceptCall'
    | 'stopCall'
    | 'enableLocalVideo'
    | 'disableLocalVideo'
    | 'sendDtmf'
    | 'cameraFirstFrameRendered';

export interface IBaseOutboundMessage {
    type : OutboundMessageType;
}

export interface IInitResponse extends IBaseOutboundMessage {
    isSupported : boolean;
}

export interface IDeviceWindowStateMessage extends IBaseOutboundMessage {
    payload : any;
}

export interface IFocusAcquireRequestMessage extends IBaseOutboundMessage {
    token : number;
    avsInterface : string;
    channelName : string;
    contentType : ContentType;
}

export interface IFocusReleaseRequestMessage extends IBaseOutboundMessage {
    token : number;
    avsInterface : string;
    channelName : string;
}

export interface IOnFocusChangedReceivedConfirmationMessage extends IBaseOutboundMessage {
    token : number;
}

export interface IRenderStaticDocumentMessage extends IBaseOutboundMessage {
    token : string;
    windowId : string;
    payload : any;
}

export interface IExecuteCommandsMessage extends IBaseOutboundMessage {
    token : string;
    payload : any;
}

export interface IAPLEventMessage extends IBaseOutboundMessage {
    windowId : string;
    payload : any;
}

export interface IActivityReportMessage extends IBaseOutboundMessage {
    event : ActivityEvent;
}

export interface INavigationReportMessage extends IBaseOutboundMessage {
    event : NavigationEvent;
}

export interface ILogEventMessage extends IBaseOutboundMessage {
    level : JSLogLevel;
    component : string;
    message : string;
}

export interface ISendDtmfMessage extends IBaseOutboundMessage {
    dtmfTone : string;
}

export const createRenderStaticDocumentMessage = (
    token : string,
    windowId : string,
    document : any,
    datasources : any,
    supportedViewports : any) : IRenderStaticDocumentMessage => {
    const payload = {
        document,
        datasources,
        supportedViewports
    };

    const message : IRenderStaticDocumentMessage = {
        type : 'renderStaticDocument',
        token,
        windowId,
        payload
    };

    return message;
};

export const createExecuteCommandsMessage = (
    token : string,
    commands : any) : IExecuteCommandsMessage => {

    const payload = {
        presentationToken : token,
        commands : [commands]
    };
    return {
        type : 'executeCommands',
        token,
        payload
    };
};
