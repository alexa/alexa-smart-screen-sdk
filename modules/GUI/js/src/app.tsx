/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import * as React from 'react';

import {
    APLRendererWindow,
    WebsocketConnectionWrapper,
    APLRendererWindowState
} from './components/APLRendererWindow';
import { SampleHome } from './components/SampleHome';
import { NavigationEvent } from './lib/messages/NavigationEvent';
import { IClientConfig, Client, IClient } from './lib/messages/client';
import {
    AlexaState,
    IAPLCoreMessage,
    IRenderStaticDocumentMessage,
    IRenderPlayerInfoMessage,
    IBaseOutboundMessage,
    IRenderTemplateMessage,
    IAlexaStateChangedMessage,
    IInitRequest,
    IInitResponse,
    IFocusAcquireRequestMessage,
    IFocusReleaseRequestMessage,
    IFocusResponseMessage,
    IOnFocusChangedMessage,
    IOnFocusChangedReceivedConfirmationMessage,
    IActivityReportMessage,
    INavigationReportMessage,
    OutboundMessageType,
    IRequestAuthorizationMessage,
    IAuthorizationChangeMessage,
    IAPLRenderMessage,
    IGuiConfigurationMessage,
    IDeviceWindowStateMessage,
    IBaseInboundMessage,
    IRenderCaptionsMessage
} from './lib/messages/messages';
import { PlayerInfoWindow, RENDER_PLAYER_INFO_WINDOW_ID } from './components/PlayerInfoWindow';
import { resolveRenderTemplate } from './lib/displayCards/AVSDisplayCardHelpers';
import { SDKLogTransport } from './lib/messages/sdkLogTransport';
import { ILogger, LoggerFactory } from 'apl-client';
import { FocusManager } from './lib/focus/FocusManager';
import { ActivityTracker } from './lib/activity/ActivityTracker';
import { ActivityEvent } from './lib/activity/ActivityEvent';
import { VoiceChrome } from './components/VoiceChrome';
import { IDeviceAppConfig, AudioInputInitiator } from './lib/config/IDeviceAppConfig';
import { IDisplayPixelDimensions } from './lib/config/visualCharacteristics/IDeviceDisplay';
import { resolveDeviceAppConfig, resolveDeviceWindowState } from './lib/config/GuiConfigHelpers';
import { IWindowState } from './lib/config/visualCharacteristics/IWindowState';
import { UWPWebViewClient } from './lib/messages/UWPClient';
import { CaptionsView } from './components/CaptionsView';

const HOST = 'localhost';
const PORT = 8933;

/// Maximum APL version supported by the runtime.
const APL_MAX_VERSION = '1.3';

/// The minimum SmartScreenSDK version required for this runtime.
const SMART_SCREEN_SDK_MIN_VERSION = '2.2';

/// Indicates whether the SDK has built with WebSocket SSL Disabled.
declare const DISABLE_WEBSOCKET_SSL : boolean;

/// Indicates whether to use the UWP client
declare const USE_UWP_CLIENT : boolean;

export interface IAppState {
    alexaState : AlexaState;
    targetWindowId : string;
    playerInfoMessage : IRenderPlayerInfoMessage;
    updateActiveAPLRendererWindow : boolean;
    captionsMessage : IRenderCaptionsMessage;
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
        this.logger.debug(`APL version: ${APL_MAX_VERSION} SDKVer: ${smartScreenSDKVer}`);

        const isSupported : boolean = (this.compareVersions(SMART_SCREEN_SDK_MIN_VERSION, smartScreenSDKVer) <= 0);
        this.sendInitResponse(isSupported, APL_MAX_VERSION);
    }

    protected handleRenderCaptions(message : IBaseInboundMessage) {
        this.captionFrame = JSON.parse(JSON.stringify((message as IRenderCaptionsMessage).payload));
        clearTimeout(this.lastCaptionTimeOutId);

        this.setState({
            captionsMessage : this.captionFrame
        });

        this.lastCaptionTimeOutId = setTimeout(this.clearCaptions, this.captionFrame.duration);
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

    protected handleAPLRender(message : IAPLRenderMessage) {
        let targetWindowId : string = message.windowId ? message.windowId : this.deviceAppConfig.defaultWindowId;
        // Setting the token on the displaying window
        this.setTokenForWindowId(message.token, targetWindowId);
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

    protected handleAlexaStateChangedMessage(message : IBaseInboundMessage) {
        const alexaStateChangedMessage : IAlexaStateChangedMessage = message as IAlexaStateChangedMessage;
        this.setState((prevState, props) => {
            return {
                alexaState: alexaStateChangedMessage.state
            };
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

    protected handleClearNonPlayerInfoWindow() {
        // Clearing the token on the displaying window
        this.setTokenForWindowId(null, this.state.targetWindowId);
        this.setState({targetWindowId : undefined});
    }

    protected handleOnFocusChanged(message : IBaseInboundMessage) {
        const focusChanged : IOnFocusChangedMessage = message as IOnFocusChangedMessage;
        // Message received, notify C++ bridge, then process
        this.sendOnFocusChangedReceivedConfirmation(focusChanged.token);
        this.focusManager.processFocusChanged(focusChanged.token, focusChanged.channelState);
    }

    protected handleGuiConfigurationMessage(message : IBaseInboundMessage) {
        const guiConfigurationMessage : IGuiConfigurationMessage
            = message as IGuiConfigurationMessage;

        this.deviceAppConfig = resolveDeviceAppConfig(
            window.innerWidth, window.innerHeight, guiConfigurationMessage.payload, this.logger);

        this.windowState = resolveDeviceWindowState(this.deviceAppConfig);

        switch (this.deviceAppConfig.audioInputInitiator) {
            case AudioInputInitiator.PRESS_AND_HOLD : {
                this.talkButtonDownMessage = 'holdToTalk';
                this.talkButtonUpMessage = 'holdToTalk';
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
            document.addEventListener('touchstart', this.handleUserInteraction.bind(this));
            document.addEventListener('mousedown', this.handleUserInteraction.bind(this));
        }

        this.sendDeviceWindowState();

        // Using setState to re render after gathering all necessary device configurations.
        this.setState({});
    }

    protected onClientMessage(message : IBaseInboundMessage) {
        switch (message.type) {
            case 'initRequest': {
                this.handleInitRequest(message);
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
                this.handleClearNonPlayerInfoWindow();
                break;
            }
            case 'aplRender': {
              this.handleAPLRender(message as IAPLRenderMessage);
              break;
            }
            case 'aplCore': {
              this.aplConnection.handleMessage(message as IAPLCoreMessage);
              break;
            }
            case 'renderCaptions': {
                this.handleRenderCaptions(message);
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

        this.state = {
            alexaState : AlexaState.IDLE,
            playerInfoMessage : undefined,
            updateActiveAPLRendererWindow : false,
            targetWindowId : undefined,
            captionsMessage : undefined
        };

        this.eventListenersAdded = false;
    }

    protected sendInitResponse(isSupported : boolean, APLMaxVersion : string) {
        const message : IInitResponse = {
            type : 'initResponse',
            isSupported,
            APLMaxVersion
        };

        this.client.sendMessage(message);
    }

    protected sendFocusAcquireRequest(channelName : string, token : number) {
        const message : IFocusAcquireRequestMessage = {
            type : 'focusAcquireRequest',
            token,
            channelName
        };
        this.client.sendMessage(message);
    }

    protected sendFocusReleaseRequest(channelName : string, token : number) {
        const message : IFocusReleaseRequestMessage = {
            type : 'focusReleaseRequest',
            token,
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
                this.state.targetWindowId === RENDER_PLAYER_INFO_WINDOW_ID
                && this.state.updateActiveAPLRendererWindow}
            windowConfig={this.deviceAppConfig.renderPlayerInfoWindowConfig}
            client={this.aplConnection}
            focusManager={this.focusManager}
            activityTracker={this.activityTracker}
          />;

        // Create APL Renderer Windows from GUI App Config
        const aplRendererWindows = this.deviceAppConfig.rendererWindowConfigs.map((window) => {
            return (
                <APLRendererWindow
                    id={window.id}
                    key={window.id}
                    windowConfig={window}
                    windowState={window.id === this.state.targetWindowId ?
                        APLRendererWindowState.ACTIVE : APLRendererWindowState.INACTIVE}
                    refreshRenderer={
                        window.id === this.state.targetWindowId
                        && this.state.updateActiveAPLRendererWindow}
                    client={this.aplConnection}
                    focusManager={this.focusManager}
                    activityTracker={this.activityTracker}
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
                {aplRendererWindows}
                <CaptionsView
                    captionsMessage={this.state.captionsMessage}
                />
                <VoiceChrome
                    deviceAppConfig={this.deviceAppConfig}
                    targetWindowId={this.state.targetWindowId}
                    alexaState={this.state.alexaState}
                />
            </div>
        );
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
            default : {
                this.handleUserInteraction();
                break;
            }
        }

        this.lastKeyDownCode = event.code;
    }

    private handleKeyUp(event : any) {
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

    private handleUserInteraction() {
        // Allow user interaction while presenting Player Info Window
        // This enables UI navigation without killing progress bar.
        if (this.state.targetWindowId
            && this.state.targetWindowId !== RENDER_PLAYER_INFO_WINDOW_ID) {
            this.activityTracker.reportInterrupted();
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
}
