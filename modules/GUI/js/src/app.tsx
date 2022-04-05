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

import * as React from 'react';

import {APLRendererWindow, WebsocketConnectionWrapper} from './components/APLRendererWindow';
import {SampleHome} from './components/SampleHome';
import {NavigationEvent} from './lib/messages/NavigationEvent';
import {Client, IClient, IClientConfig} from './lib/messages/client';
import {
    AlexaState,
    CallState,
    CameraState,
    IActivityReportMessage,
    IAlexaStateChangedMessage,
    IAPLCoreMessage,
    IAPLRenderMessage,
    IAuthorizationChangeMessage,
    IBaseInboundMessage,
    IBaseOutboundMessage,
    ICallStateChangeMessage,
    ICameraStateChangedMessage,
    IClearDocumentMessage,
    IDeviceWindowStateMessage,
    IDoNotDisturbSettingChangedMessage,
    IDtmfTonesSentMessage,
    IFocusAcquireRequestMessage,
    IFocusReleaseRequestMessage,
    IFocusResponseMessage,
    IGuiConfigurationMessage,
    IInitRequest,
    IInitResponse,
    ILocaleChangeMessage,
    INavigationReportMessage,
    IOnFocusChangedMessage,
    IOnFocusChangedReceivedConfirmationMessage,
    IRenderCameraMessage,
    IRenderCaptionsMessage,
    IRenderPlayerInfoMessage,
    IRenderStaticDocumentMessage,
    IRenderTemplateMessage,
    IRequestAuthorizationMessage,
    IVideoCallingConfigMessage,
    OutboundMessageType
} from './lib/messages/messages';
import {PlayerInfoWindow} from './components/PlayerInfoWindow';
import {LiveViewCameraWindow} from './components/LiveViewCameraWindow';
import {resolveRenderTemplate} from './lib/displayCards/AVSDisplayCardHelpers';
import {CommsWindow, ICommsParameters} from './components/CommsWindow';
import {SDKLogTransport} from './lib/messages/sdkLogTransport';
import {ILogger, LoggerFactory} from 'apl-client';
import {FocusManager} from './lib/focus/FocusManager';
import {ActivityTracker} from './lib/activity/ActivityTracker';
import {ActivityEvent} from './lib/activity/ActivityEvent';
import {VoiceChrome} from './components/VoiceChrome';
import {AudioInputInitiator, IDeviceAppConfig} from './lib/config/IDeviceAppConfig';
import {IDisplayPixelDimensions} from './lib/config/visualCharacteristics/IDeviceDisplay';
import {resolveDeviceAppConfig, resolveDeviceWindowState} from './lib/config/GuiConfigHelpers';
import {IWindowState} from './lib/config/visualCharacteristics/IWindowState';
import {UWPWebViewClient} from './lib/messages/UWPClient';
import {CaptionsView} from './components/CaptionsView';
import {LocaleManager} from './lib/utils/localeManager';
import {ContentType} from './lib/focus/ContentType';

const HOST = 'localhost';
const PORT = 8933;

/// The minimum SmartScreenSDK version required for this runtime.
const SMART_SCREEN_SDK_MIN_VERSION = '2.9.1';

/// Indicates whether the SDK has built with WebSocket SSL Disabled.
declare const DISABLE_WEBSOCKET_SSL : boolean;

/// Indicates whether to use the UWP client
declare const USE_UWP_CLIENT : boolean;

export interface IAppState {
    alexaState : AlexaState;
    callStateInfo : ICallStateChangeMessage;
    targetWindowId : string;
    playerInfoMessage : IRenderPlayerInfoMessage;
    updateActiveAPLRendererWindow : boolean;
    captionsMessage : IRenderCaptionsMessage;
    doNotDisturbSettingEnabled : boolean;
    renderCameraMessage : IRenderCameraMessage;
    cameraState : CameraState;
    clearWindow : boolean;
    videoCallingConfig : string;
    commsParameters : ICommsParameters;
}

export class App extends React.Component<any, IAppState> {
    protected rootDiv : HTMLElement;
    protected deviceAppConfig : IDeviceAppConfig;
    protected windowState : IWindowState;
    protected client : IClient;
    protected aplConnection : WebsocketConnectionWrapper;
    protected logger : ILogger;
    protected focusManager : FocusManager;
    protected activityTracker : ActivityTracker;
    protected talkButtonDownMessage : OutboundMessageType;
    protected talkButtonUpMessage : OutboundMessageType;
    protected eventListenersAdded : boolean;
    private captionFrame : any;
    private lastCaptionTimeOutId : number;
    private toggleCaptionsMessage : OutboundMessageType = 'toggleCaptions';
    private toggleDoNotDisturbMessage : OutboundMessageType = 'toggleDoNotDisturb';
    private lastRenderedWindowId : string;
    private aplCoreMessageHandlers : Map<string, (message : IAPLCoreMessage) => void> = new Map();

    /**
     * Compare two versions as strings.
     *
     * @param v1 First version to compare.
     * @param v2 Second version to compare.
     * @return n, where n<0 if v1<v2, n=0 if v1=v2 and n>0 if v1>v2
     */
    protected compareVersions(v1 : string, v2 : string) : number {
        if (!v1) {
            return -1;
        }

        if (!v2) {
            return 1;
        }

        const v1Arr : string[] = v1.split('.');
        const v2Arr : string[] = v2.split('.');

        for (let i = 0; i < Math.min(v1Arr.length, v2Arr.length); i++) {
            if (Number(v1Arr[i]) < Number(v2Arr[i])) {
                return -1;
            } else if (Number(v1Arr[i]) > Number(v2Arr[i])) {
                return 1;
            }
        }

        // The longest one is bigger
        return v1Arr.length - v2Arr.length;
    }

    protected handleInitRequest(message : IBaseInboundMessage) {
        const initRequestMessage : IInitRequest = message as IInitRequest;
        this.logger.debug(`message: ${JSON.stringify(initRequestMessage)}`);
        let smartScreenSDKVer = initRequestMessage.smartScreenSDKVersion;

        const isSupported : boolean = (this.compareVersions(SMART_SCREEN_SDK_MIN_VERSION, smartScreenSDKVer) <= 0);
        this.sendInitResponse(isSupported);
    }

    protected handleVideoCallingConfigMessage(message : IBaseInboundMessage) {
        const videoCallingConfigMessage : IVideoCallingConfigMessage = message as IVideoCallingConfigMessage;
        this.setState({
            videoCallingConfig : videoCallingConfigMessage.videoCallingConfig
        });
    }

    protected handleRenderCaptions(message : IBaseInboundMessage) {
        this.captionFrame = JSON.parse(JSON.stringify((message as IRenderCaptionsMessage).payload));
        clearTimeout(this.lastCaptionTimeOutId);

        this.setState({
            captionsMessage : this.captionFrame
        });

        this.lastCaptionTimeOutId = window.setTimeout(this.clearCaptions, this.captionFrame.duration);
    }

    protected handleDoNotDisturbSettingChanged(message : IBaseInboundMessage) {
        const newDoNotDisturbSettingEnabled =
            (message as IDoNotDisturbSettingChangedMessage).doNotDisturbSettingEnabled;

        this.setState({
            doNotDisturbSettingEnabled : newDoNotDisturbSettingEnabled
        });
    }

    protected clearCaptions = () => {
        this.setState({
            captionsMessage : undefined
        });
    }

    protected handleRenderTemplateMessage(message : IBaseInboundMessage) {
        const renderTemplateMessage : IRenderTemplateMessage = message as IRenderTemplateMessage;
        const renderStaticDocumentMessage : IRenderStaticDocumentMessage =
            resolveRenderTemplate(renderTemplateMessage, this.deviceAppConfig.renderTemplateWindowId);

        this.client.sendMessage(renderStaticDocumentMessage);
    }

    protected handleRenderPlayerInfoMessage(message : IBaseInboundMessage) {
        const renderPlayerInfoMessage : IRenderPlayerInfoMessage = message as IRenderPlayerInfoMessage;
        this.setState({
            playerInfoMessage : renderPlayerInfoMessage
        });
    }

    protected handleRenderCameraMessage(message : IBaseInboundMessage) {
        const cameraMessage : IRenderCameraMessage = message as IRenderCameraMessage;
        this.setState({
            renderCameraMessage : cameraMessage
        });
    }

    protected handleClearCameraMessage() {
        this.setState({
            renderCameraMessage : undefined
        });
    }

    protected handleCameraStateChanged(message : IBaseInboundMessage) {
        const cameraMessage : ICameraStateChangedMessage = message as ICameraStateChangedMessage;
        this.setState({
            cameraState : cameraMessage.cameraState
        });
    }

    protected handleAPLRender(message : IAPLRenderMessage) {
        let targetWindowId : string = message.windowId ? message.windowId : this.deviceAppConfig.defaultWindowId;
        // Setting the token on the displaying window
        this.setTokenForWindowId(message.token, targetWindowId);
        this.lastRenderedWindowId = targetWindowId;

        this.setState((prevState, props) => ({
                targetWindowId,
                updateActiveAPLRendererWindow: true
            })
        );
        // Make sure the active window only updates once.
        this.setState({
            updateActiveAPLRendererWindow: false
        });
    }

    protected handleAPLCore(message : IAPLCoreMessage) {
        const windowId : string = message.windowId ? message.windowId : this.deviceAppConfig.defaultWindowId;

        let aplCoreMessageHandler = this.aplCoreMessageHandlers.get(windowId);
        if (aplCoreMessageHandler !== undefined) {
            aplCoreMessageHandler(message);
        }
    }

    protected handleAlexaStateChangedMessage(message : IBaseInboundMessage) {
        const alexaStateChangedMessage : IAlexaStateChangedMessage = message as IAlexaStateChangedMessage;
        this.setState((prevState, props) => {
            return {
                alexaState: alexaStateChangedMessage.state
            };
        });
    }

    protected handleCallStateChangeMessage(message : IBaseInboundMessage) {
        const callStateChangeMessage : ICallStateChangeMessage = message as ICallStateChangeMessage;

        this.setState((prevState, props) => {
            return {
                callStateInfo: callStateChangeMessage
            };
        });
    }

    protected handleDtmfTonesSentMessage(dtmfTonesSentMessage : IDtmfTonesSentMessage) {
        this.setState({
            commsParameters : {
                dtmfTones : dtmfTonesSentMessage.dtmfTones,
                dtmfUpdate : 1 - this.state.commsParameters.dtmfUpdate
            }
          });
    }

    protected handleRequestAuthorization(requestAuthorizationMessage : IRequestAuthorizationMessage) {
        /**
         * Use to present CBL authorization.
         * API :
         * https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html
         * Design Guidance :
         * https://developer.amazon.com/docs/alexa-voice-service/setup-authentication.html#code-based-screens
         */
    }

    protected handleAuthorizationStateChanged(authStateChangeMessage : IAuthorizationChangeMessage) {
        // Use to drive app behavior based on authorization state changes.
    }

    protected handleFocusResponse(message : IBaseInboundMessage) {
        const focusResponse : IFocusResponseMessage = message as IFocusResponseMessage;
        this.focusManager.processFocusResponse(focusResponse.token, focusResponse.result);
    }

    protected handleClearPlayerInfoWindow() {
        this.setState({
            targetWindowId : undefined,
            playerInfoMessage : undefined
        });
    }

    protected handleClearNonPlayerInfoWindow(message : IClearDocumentMessage) {
        // Clearing the token on the displaying window
        this.setTokenForWindowId(null, this.state.targetWindowId);
        this.setState({
            targetWindowId : message.windowId,
            clearWindow: true
        });

        // Make sure clearWindow only updates once.
        this.setState({
            targetWindowId : undefined,
            clearWindow: false
        });
    }

    protected handleOnFocusChanged(message : IBaseInboundMessage) {
        const focusChanged : IOnFocusChangedMessage = message as IOnFocusChangedMessage;
        // Message received, notify C++ bridge, then process
        this.sendOnFocusChangedReceivedConfirmation(focusChanged.token);
        this.focusManager.processFocusChanged(focusChanged.token, focusChanged.focusState);
    }

    protected handleGuiConfigurationMessage(message : IBaseInboundMessage) {
        const guiConfigurationMessage : IGuiConfigurationMessage
            = message as IGuiConfigurationMessage;

        this.deviceAppConfig = resolveDeviceAppConfig(
            window.innerWidth, window.innerHeight, guiConfigurationMessage.payload, this.logger);

        this.windowState = resolveDeviceWindowState(this.deviceAppConfig);

        switch (this.deviceAppConfig.audioInputInitiator) {
            case AudioInputInitiator.PRESS_AND_HOLD : {
                this.talkButtonDownMessage = 'holdToTalkStart';
                this.talkButtonUpMessage = 'holdToTalkEnd';
                break;
            }
            case AudioInputInitiator.TAP : {
                this.talkButtonDownMessage = 'tapToTalk';
                this.talkButtonUpMessage = undefined;
                break;
            }
            default : {
                break;
            }
        }
        if (!this.eventListenersAdded) {
            this.eventListenersAdded = true;
            document.addEventListener('keydown', this.handleKeyDown.bind(this));
            document.addEventListener('keyup', this.handleKeyUp.bind(this));

            document.addEventListener('touchstart', this.handleUserInterruption.bind(this));
            document.addEventListener('mousedown', this.handleUserInterruption.bind(this));
            document.addEventListener('wheel', this.handleUserInterruption.bind(this), {capture: true, passive: true});
        }

        this.sendDeviceWindowState();

        // Using setState to re render after gathering all necessary device configurations.
        this.setState({});
    }

    protected handleLocaleChangeMessage(message : ILocaleChangeMessage) {
        /**
         * Use to implement UI changes when Alexa locale changes:
         * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/system.html#setlocales
         */
        LocaleManager.setLocales(message.locales);
    }

    protected onClientMessage(message : IBaseInboundMessage) {
        switch (message.type) {
            case 'initRequest': {
                this.handleInitRequest(message);
                break;
            }
            case 'videoCallingConfig': {
                this.handleVideoCallingConfigMessage(message);
                break;
            }
            case 'guiConfiguration': {
                this.handleGuiConfigurationMessage(message);
                break;
            }
            case 'requestAuthorization' : {
                this.handleRequestAuthorization(message as IRequestAuthorizationMessage);
                break;
            }
            case 'authorizationChange' : {
                this.handleAuthorizationStateChanged(message as IAuthorizationChangeMessage);
                break;
            }
            case 'alexaStateChanged': {
                this.handleAlexaStateChangedMessage(message);
                break;
            }
            case 'callStateChange': {
                this.handleCallStateChangeMessage(message);
                break;
            }
            case 'dtmfTonesSent': {
                this.handleDtmfTonesSentMessage(message as IDtmfTonesSentMessage);
                break;
            }
            case 'focusResponse': {
                this.handleFocusResponse(message);
                break;
            }
            case 'onFocusChanged' : {
                this.handleOnFocusChanged(message);
                break;
            }
            case 'renderTemplate': {
                this.handleRenderTemplateMessage(message);
                break;
            }
            case 'renderPlayerInfo': {
                this.handleRenderPlayerInfoMessage(message);
                break;
            }
            case 'clearPlayerInfoCard': {
                this.handleClearPlayerInfoWindow();
                break;
            }
            case 'clearDocument':
            case 'clearTemplateCard': {
                this.handleClearNonPlayerInfoWindow(message as IClearDocumentMessage);
                break;
            }
            case 'aplRender': {
              this.handleAPLRender(message as IAPLRenderMessage);
              break;
            }
            case 'aplCore': {
              this.handleAPLCore(message as IAPLCoreMessage);
              break;
            }
            case 'renderCaptions': {
                this.handleRenderCaptions(message);
                break;
            }
            case 'renderCamera': {
                this.handleRenderCameraMessage(message as IRenderCameraMessage);
                break;
            }
            case 'clearCamera': {
                this.handleClearCameraMessage();
                break;
            }
            case 'cameraStateChanged': {
                this.handleCameraStateChanged(message as ICameraStateChangedMessage);
                break;
            }
            case 'doNotDisturbSettingChanged': {
                this.handleDoNotDisturbSettingChanged(message);
                break;
            }
            case 'localeChange': {
                this.handleLocaleChangeMessage(message as ILocaleChangeMessage);
                break;
            }
            default: {
                this.logger.warn('received message with unsupported type. Type: ', message.type);
                break;
            }
        }
    }

    constructor(props : any) {
        super(props);
        // NOTE: No logging should happen before here!
        LoggerFactory.initialize('info', SDKLogTransport.logFunction);
        this.logger = LoggerFactory.getLogger('App');
        this.rootDiv = document.getElementById('root');
        this.focusManager = new FocusManager({
            acquireFocus: this.sendFocusAcquireRequest.bind(this),
            releaseFocus: this.sendFocusReleaseRequest.bind(this)
        });
        this.activityTracker = new ActivityTracker(this.sendActivityEvent.bind(this));

        const clientConfig : IClientConfig = {
            host : HOST,
            port : PORT,
            onMessage : this.onClientMessage.bind(this),
            insecure : DISABLE_WEBSOCKET_SSL
        };
        if (USE_UWP_CLIENT) {
            this.client = new UWPWebViewClient(clientConfig);
        } else {
            this.client = new Client(clientConfig);
        }
        this.aplConnection = new WebsocketConnectionWrapper(this.client);
        SDKLogTransport.initialize(this.client);
        const callStateMessage : ICallStateChangeMessage = {
            type: undefined,
            callState: CallState.NONE,
            callType: undefined,
            previousSipUserAgentState: undefined,
            currentSipUserAgentState: undefined,
            displayName: undefined,
            endpointLabel: undefined,
            inboundCalleeName: undefined,
            callProviderType: undefined,
            inboundRingtoneUrl: undefined,
            outboundRingbackUrl: undefined,
            isDropIn: false
        };

        this.state = {
            alexaState : AlexaState.IDLE,
            callStateInfo : callStateMessage,
            playerInfoMessage : undefined,
            updateActiveAPLRendererWindow : false,
            targetWindowId : undefined,
            captionsMessage : undefined,
            renderCameraMessage: undefined,
            cameraState: CameraState.UNKNOWN,
            doNotDisturbSettingEnabled : undefined,
            clearWindow: false,
            videoCallingConfig: undefined,
            commsParameters : {
                dtmfTones : undefined,
                dtmfUpdate : 0
            }
        };

        this.eventListenersAdded = false;
    }

    protected sendInitResponse(isSupported : boolean) {
        const message : IInitResponse = {
            type : 'initResponse',
            isSupported
        };

        this.client.sendMessage(message);
    }

    protected sendFocusAcquireRequest(
        avsInterface : string, channelName : string, contentType : ContentType, token : number) {
        const message : IFocusAcquireRequestMessage = {
            type : 'focusAcquireRequest',
            token,
            avsInterface,
            channelName,
            contentType
        };
        this.client.sendMessage(message);
    }

    protected sendFocusReleaseRequest(avsInterface : string, channelName : string, token : number) {
        const message : IFocusReleaseRequestMessage = {
            type : 'focusReleaseRequest',
            token,
            avsInterface,
            channelName
        };
        this.client.sendMessage(message);
    }

    protected sendOnFocusChangedReceivedConfirmation(token : number) {
        const message : IOnFocusChangedReceivedConfirmationMessage = {
            type : 'onFocusChangedReceivedConfirmation',
            token
        };
        this.client.sendMessage(message);
    }

    protected sendTalkButtonEvent(type : OutboundMessageType) {
        if (type === undefined) {
            return;
        }
        const message : IBaseOutboundMessage = {
            type
        };
        this.client.sendMessage(message);
    }

    protected sendToggleCaptionsEvent(type : OutboundMessageType) {
        if (type === undefined) {
            return;
        }
        const message : IBaseOutboundMessage = {
            type
        };
        this.client.sendMessage(message);
    }

    protected sendToggleDoNotDisturbEvent(type : OutboundMessageType) {
        if (type === undefined) {
            return;
        }
        const message : IBaseOutboundMessage = {
            type
        };
        this.client.sendMessage(message);
    }

    protected sendActivityEvent(event : ActivityEvent) {
        if (this.state.targetWindowId === undefined) {
            return;
        }
        const message : IActivityReportMessage = {
            type : 'activityEvent',
            event
        };
        this.client.sendMessage(message);
    }

    protected sendNavigationEvent(event : NavigationEvent) {
        const message : INavigationReportMessage = {
            type : 'navigationEvent',
            event
        };
        this.client.sendMessage(message);
    }

    protected sendDeviceWindowState() {
        const deviceWindowStateMessage : IDeviceWindowStateMessage = {
            type : 'deviceWindowState',
            payload : this.windowState
        };
        this.client.sendMessage(deviceWindowStateMessage);
    }

    public render() {
        if (!this.deviceAppConfig) {
            return (
                <div id='displayContainer'>
                    <SampleHome/>
                </div>
            );
        }
        // PlayerInfo Window
        const playerInfo = <PlayerInfoWindow
            playerInfoMessage={this.state.playerInfoMessage}
            targetWindowId={this.state.targetWindowId}
            refreshRenderer={
                this.state.targetWindowId === this.deviceAppConfig.renderPlayerInfoWindowConfig.id &&
                this.state.updateActiveAPLRendererWindow}
            windowConfig={this.deviceAppConfig.renderPlayerInfoWindowConfig}
            client={this.aplConnection}
            focusManager={this.focusManager}
            activityTracker={this.activityTracker}
            aplCoreMessageHandlerCallback={this.setAPLCoreMessageHandler.bind(this)}
          />;

        // LiveViewCameraWindow
        const liveViewCameraWindow = <LiveViewCameraWindow
            renderCameraMessage={this.state.renderCameraMessage}
            cameraState={this.state.cameraState}
            refreshRenderer={
                this.state.targetWindowId === this.deviceAppConfig.liveViewUIWindowConfig.id &&
                this.state.updateActiveAPLRendererWindow}
            windowConfig={this.deviceAppConfig.liveViewUIWindowConfig}
            client={this.aplConnection}
            aplCoreMessageHandlerCallback={this.setAPLCoreMessageHandler.bind(this)}
            focusManager={this.focusManager}
            activityTracker={this.activityTracker}
        />;

        // Comms Window
        const commsWindow = <CommsWindow
            callStateInfo = {this.state.callStateInfo}
            client={this.client}
            locale={LocaleManager.getPrimaryLocale()}
            videoCallingConfig={this.state.videoCallingConfig}
            commsParameters={this.state.commsParameters}
            />;

        // Create APL Renderer Windows from GUI App Config
        const aplRendererWindows = this.deviceAppConfig.rendererWindowConfigs.map((window) => {
            return (
                <APLRendererWindow
                    id={window.id}
                    key={window.id}
                    windowConfig={window}
                    clearRenderer={
                        window.id === this.state.targetWindowId
                        && this.state.clearWindow}
                    refreshRenderer={
                        window.id === this.state.targetWindowId
                        && this.state.updateActiveAPLRendererWindow}
                    client={this.aplConnection}
                    focusManager={this.focusManager}
                    activityTracker={this.activityTracker}
                    aplCoreMessageHandlerCallback={this.setAPLCoreMessageHandler.bind(this)}
                />
            );
        });

        return (
            <div id='displayContainer'
                style={this.getDisplayStyle()}>
                <SampleHome
                    deviceAppConfig={this.deviceAppConfig}
                />
                {playerInfo}
                {liveViewCameraWindow}
                {commsWindow}
                {aplRendererWindows}
                <VoiceChrome
                    deviceAppConfig={this.deviceAppConfig}
                    targetWindowId={this.state.targetWindowId}
                    alexaState={this.state.alexaState}
                    doNotDisturbSettingEnabled={this.state.doNotDisturbSettingEnabled}
                />
                <CaptionsView
                    captionsMessage={this.state.captionsMessage}
                />
            </div>
        );
    }

    public setAPLCoreMessageHandler(windowId : string, aplCoreMessageHandler : (message : IAPLCoreMessage) => void) {
        this.aplCoreMessageHandlers.set(windowId, aplCoreMessageHandler);
    }

    protected getDisplayStyle() : any {
        let scale = 1;
        let height : any = '100%';
        let width : any = '100%';
        let clipPath : string = 'none';

        if (this.deviceAppConfig.emulateDisplayDimensions) {

            const displayPixelDimensions =
                this.deviceAppConfig.display.dimensions as IDisplayPixelDimensions;

            height = displayPixelDimensions.resolution.value.height;
            width = displayPixelDimensions.resolution.value.width;
            clipPath = this.deviceAppConfig.display.shape === 'ROUND' ? 'circle(50%)' : clipPath;

            if (this.deviceAppConfig.scaleToFill) {
                scale = Math.min(window.innerWidth / width, window.innerHeight / height);
            }
        }

        const displayStyle = {
            height,
            width,
            transform : 'scale(' + scale + ')',
            clipPath
        };
        return displayStyle;
    }

    protected lastKeyDownCode : string;

    private handleKeyDown(event : any) {
        if (USE_UWP_CLIENT) {
            this.convertUwpKeyboardEvent(event);
        }

        // Only handle key down events once
        if (event.code === this.lastKeyDownCode) {
            return;
        }
        switch (event.code) {
            // Press talk key to start audio recognition
            case this.deviceAppConfig.deviceKeys.talkKey.code : {
                this.sendTalkButtonEvent(this.talkButtonDownMessage);
                break;
            }
            // Similar to EXIT button pressed on remote
            case this.deviceAppConfig.deviceKeys.exitKey.code:
                this.sendNavigationEvent(NavigationEvent.EXIT);
                break;
            // Similar to BACK button pressed on remote
            case this.deviceAppConfig.deviceKeys.backKey.code:
                this.sendNavigationEvent(NavigationEvent.BACK);
                break;
            // Similar to toggle options setting on remote
            case this.deviceAppConfig.deviceKeys.toggleCaptionsKey.code:
                this.sendToggleCaptionsEvent(this.toggleCaptionsMessage);
                break;
            // Similar to toggle Do Not Disturb setting on remote
            case this.deviceAppConfig.deviceKeys.toggleDoNotDisturbKey.code:
                this.sendToggleDoNotDisturbEvent(this.toggleDoNotDisturbMessage);
                break;
            // All KeyDown events trigger user interruption
            default : {
                this.handleUserInterruption();
                break;
            }
        }

        this.lastKeyDownCode = event.code;
    }

    private handleKeyUp(event : any) {
        if (USE_UWP_CLIENT) {
            this.convertUwpKeyboardEvent(event);
        }

        this.lastKeyDownCode = undefined;

        switch (event.code) {
            // Release talk key to stop audio recognition on PRESS_AND_HOLD integrations
            case this.deviceAppConfig.deviceKeys.talkKey.code: {
                this.sendTalkButtonEvent(this.talkButtonUpMessage);
                break;
            }
            default : {
                break;
            }
        }
    }

    /**
     * Boolean tracking locked state of user interruption event handling.
     */
    private interruptionLock = false;

    /** The interrupted event is reported when an user interaction interrupts an activity.
     *  Examples include keyDown events, clicks, touches, and scrolls.
     *  Some of these user interactions results in multiple events per user interaction, such
     *  as scrolling. Even when multiple events are fired, the interrupted event only needs to be
     *  reported once in a set time period. This implementation will report a maximum of one
     *  interrupted activity every 500 ms.
     */
    private handleUserInterruption() {
        if (!this.interruptionLock) {
            this.interruptionLock = true;
            this.activityTracker.reportInterrupted();
            window.setTimeout(() => {
                this.interruptionLock = false;
            }, 500);
        }
    }

    private setTokenForWindowId(token : string, windowId : string) {
        for (let window of this.windowState.instances) {
            if (window.id === windowId) {
                window.token = token;
                break;
            }
        }

        this.sendDeviceWindowState();
    }

    public componentDidMount() {
        this.client.connect();
    }

    private convertUwpKeyboardEvent(event : any) {
        /**
         * Since KeyboardEvents emitted by UWP WebView do not have code property populated,
         * we have to populate them manually
         */
        switch (event.key) {
            case this.deviceAppConfig.deviceKeys.talkKey.key :
                event.code = this.deviceAppConfig.deviceKeys.talkKey.code;
                break;
            case this.deviceAppConfig.deviceKeys.exitKey.key:
                event.code = this.deviceAppConfig.deviceKeys.exitKey.code;
                break;
            case this.deviceAppConfig.deviceKeys.backKey.key:
                event.code = this.deviceAppConfig.deviceKeys.backKey.code;
                break;
            case this.deviceAppConfig.deviceKeys.toggleCaptionsKey.key:
                event.code = this.deviceAppConfig.deviceKeys.toggleCaptionsKey.code;
                break;
            case this.deviceAppConfig.deviceKeys.toggleDoNotDisturbKey.key:
                event.code = this.deviceAppConfig.deviceKeys.toggleDoNotDisturbKey.code;
                break;
            default : {
                break;
            }
        }
    }
}
