/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { ActivityEvent } from '../activity/ActivityEvent';
import { JSLogLevel } from 'apl-client';
import { NavigationEvent } from './NavigationEvent';

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
    | 'ERROR';

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
    | 'doNotDisturbSettingChanged';

export interface IBaseInboundMessage {
    type : InboundMessageType;
}

export interface IInitRequest extends IBaseInboundMessage {
    smartScreenSDKVersion : string;
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
    channelState : string;
}

export interface IFocusResponseMessage extends IBaseInboundMessage {
    token : number;
    result : boolean;
}

export interface IRenderTemplateMessage extends IBaseInboundMessage {
    payload : any;
}

export interface IRenderPlayerInfoMessage extends IBaseInboundMessage {
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

export interface IDoNotDisturbSettingChangedMessage extends IBaseInboundMessage {
    doNotDisturbSettingEnabled : boolean;
}

export type OutboundMessageType =
    'initResponse'
    | 'deviceWindowState'
    | 'focusAcquireRequest'
    | 'focusReleaseRequest'
    | 'onFocusChangedReceivedConfirmation'
    | 'tapToTalk'
    | 'holdToTalk'
    | 'renderStaticDocument'
    | 'executeCommands'
    | 'aplEvent'
    | 'activityEvent'
    | 'navigationEvent'
    | 'logEvent'
    | 'toggleCaptions'
    | 'toggleDoNotDisturb';

export interface IBaseOutboundMessage {
    type : OutboundMessageType;
}

export interface IInitResponse extends IBaseOutboundMessage {
    isSupported : boolean;
    APLMaxVersion : string;
}

export interface IDeviceWindowStateMessage extends IBaseOutboundMessage {
    payload : any;
}

export interface IFocusAcquireRequestMessage extends IBaseOutboundMessage {
    token : number;
    channelName : string;
}

export interface IFocusReleaseRequestMessage extends IBaseOutboundMessage {
    token : number;
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

    return {
        type : 'renderStaticDocument',
        token,
        windowId,
        payload
    };
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
