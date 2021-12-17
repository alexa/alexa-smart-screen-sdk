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
import {IAPLRendererWindowConfig} from '../lib/config/IDeviceAppConfig';
import {
    CameraState,
    createRenderStaticDocumentMessage,
    IAPLCoreMessage, IBaseOutboundMessage,
    IRenderCameraMessage
} from '../lib/messages/messages';
import {APLRendererWindow, WebsocketConnectionWrapper} from './APLRendererWindow';
import * as LiveViewCameraTemplate from '../lib/liveView/LiveViewUI.json';
import * as LiveViewCameraSupportedViewports from '../lib/liveView/LiveViewSupportedViewports.json';
import {LocaleManager} from '../lib/utils/localeManager';
import {FocusManager} from '../lib/focus/FocusManager';
import {FocusState} from '../lib/focus/FocusState';
import {ChannelName} from '../lib/focus/ChannelName';
import {AVSInterface} from '../lib/focus/AVSInterface';
import {ContentType} from '../lib/focus/ContentType';
import {ActivityTracker} from '../lib/activity/ActivityTracker';

type VisibilityProperty = import('csstype').Property.Visibility;

const WINDOW_TRANSITION_IN_MS : number = 250;

/// Indicates whether to use RTCSC
declare const RTCSC_ENABLED : boolean;

interface ILiveViewCameraWindowProps {
    renderCameraMessage : IRenderCameraMessage;
    cameraState : CameraState;
    refreshRenderer : boolean;
    windowConfig : IAPLRendererWindowConfig;
    client : WebsocketConnectionWrapper;
    aplCoreMessageHandlerCallback :
        (windowId : string, aplCoreMessageHandler : (message : IAPLCoreMessage) => void) => void;
    focusManager : FocusManager;
    activityTracker : ActivityTracker;
}

enum UIState {
    LOADING,
    RENDERED,
    CLEARED
}

enum SpeakerState {
    // Camera speaker is unmuted and active by default.
    UNMUTED = 'UNMUTED',
    // Camera speaker is muted and inactive by default.
    MUTED = 'MUTED',
    // Camera speaker is disabled or nonexistent.
    DISABLED = 'DISABLED'
}

interface ILiveViewCameraWindowState {
    cameraState : CameraState;
    uiState : UIState;
    initialSpeakerState : SpeakerState;
    firstFrameRendered : boolean;
}

export class LiveViewCameraWindow extends React.Component<ILiveViewCameraWindowProps, ILiveViewCameraWindowState> {
    private readonly videoRef : React.RefObject<HTMLVideoElement>;
    protected client : WebsocketConnectionWrapper;
    protected windowConfig : IAPLRendererWindowConfig;
    private rtcAdapter : any;
    private mediaClient : any;
    private mediaClientInitialized : boolean;
    private uiLoading : boolean;
    private focusToken : number = 0;
    private focusState : FocusState;
    private liveViewFocusResolver : { resolve : Function, reject : Function };
    private activityToken : number = 0;

    constructor(props : ILiveViewCameraWindowProps) {
        super(props);
        this.videoRef = React.createRef();
        this.client = this.props.client;
        this.windowConfig = this.props.windowConfig;

        this.state = {
            cameraState : CameraState.DISCONNECTED,
            uiState : UIState.CLEARED,
            initialSpeakerState : SpeakerState.DISABLED,
            firstFrameRendered : false
        };
    }

    public async setActive() {
        if (!RTCSC_ENABLED) {
            return;
        }
        if (!this.rtcAdapter) {
            this.rtcAdapter = await import('@amzn/rtcmedia_browser_adapter');
        }
        if (!this.mediaClient) {
            this.mediaClient = this.rtcAdapter.getMediaClient();
        }
        if (this.mediaClientInitialized) {
            this.mediaClient.shutdown();
        }
        this.mediaClient.initialize();
        this.mediaClientInitialized = true;

        const document = LiveViewCameraTemplate as any;
        document.environment = {
            lang: LocaleManager.getPrimaryLocale(),
            layoutDirection: LocaleManager.getLocaleLayoutDirection()
        };

        const liveViewCameraUIDocumentMessage = createRenderStaticDocumentMessage(
            this.props.windowConfig.id,
            this.props.windowConfig.id,
            document,
            {
                liveView: this.props.renderCameraMessage.payload,
                options: this.props.renderCameraMessage.liveViewControllerOptions
            },
            LiveViewCameraSupportedViewports
        );
        this.uiLoading = false;
        this.client.sendMessage(liveViewCameraUIDocumentMessage);
        this.setState({
            cameraState : CameraState.CONNECTING,
            initialSpeakerState :
            this.props.renderCameraMessage.payload.viewerExperience.audioProperties.speakerState
        });
    }

    protected async handleCameraStateChanged(cameraState : CameraState) {
        if (CameraState.CONNECTED === cameraState) {
            this.mediaClient.getRemoteStream().then((mediaStream : MediaStream) => {
                this.acquireFocus().then(() => {
                    const videoElement = this.videoRef.current;
                    videoElement.onplaying = this.videoStarted.bind(this);
                    videoElement.onpause = this.videoStopped.bind(this);
                    videoElement.onended = this.videoStopped.bind(this);
                    videoElement.onerror = this.videoStopped.bind(this);
                    videoElement.muted = !this.shouldUnmuteVideo();
                    if (videoElement.srcObject !== mediaStream) {
                        videoElement.srcObject = mediaStream;
                    }
                    videoElement.play();
                });
            });
        }
        this.setState({cameraState});
    }

    public render() {
        if (RTCSC_ENABLED) {
            const windowTransition : string = `visibility 1s, opacity ${WINDOW_TRANSITION_IN_MS}ms linear`;
            const videoTransition : string = `opacity ${WINDOW_TRANSITION_IN_MS}ms linear`;
            const videoWidth = this.props.windowConfig.viewport.shape === 'ROUND' ? '85%' : '100%';
            let windowVisibility : VisibilityProperty = 'hidden';
            let windowOpacity = 0;
            let videoVisibility : VisibilityProperty = 'hidden';
            let videoOpacity = 0;

            // If we have an LVC message payload, we can become visible
            if (this.props.renderCameraMessage) {
                windowVisibility = 'visible';
                windowOpacity = 1;

                // If the UI layer has rendered, we can make video visible
                if (this.state.uiState === UIState.RENDERED && this.state.firstFrameRendered) {
                    // Unmute the video if required for init
                    if (this.videoRef.current && this.shouldUnmuteVideo()) {
                        this.videoRef.current.muted = false;
                    }
                    videoVisibility = 'visible';
                    videoOpacity = 1;
                }
            }

            const videoItem = <div style={{
                height: '100%',
                width: '100%',
                position: 'absolute',
                display: 'flex',
                justifyContent: 'center',
                alignItems: 'center'
            }}>
                <video ref={this.videoRef} style={{
                    height: '100%',
                    width: videoWidth,
                    visibility: videoVisibility,
                    opacity: videoOpacity,
                    transition: videoTransition
                }}/>
            </div>;

            const liveViewCameraUI = <APLRendererWindow
                id={this.windowConfig.id}
                windowConfig={this.windowConfig}
                clearRenderer={this.state.cameraState === CameraState.DISCONNECTED}
                refreshRenderer={!this.uiLoading && this.state.uiState === UIState.LOADING}
                client={this.client}
                aplCoreMessageHandlerCallback={this.props.aplCoreMessageHandlerCallback}
                backgroundColorOverride={'transparent'}
                onRendererInit={this.uiRendered.bind(this)}
                onRendererDestroyed={this.uiDestroyed.bind(this)}
            />;
            if (this.state.uiState === UIState.LOADING && !this.uiLoading) {
                this.uiLoading = true;
            }

            return (
                <div style={{
                    height: '100%',
                    width: '100%',
                    position: 'absolute',
                    backgroundColor: 'black',
                    visibility: windowVisibility,
                    opacity: windowOpacity,
                    transition: windowTransition
                }}
                >
                    {videoItem}
                    {liveViewCameraUI}
                </div>
            );
        }
        // Do not render anything if RTCSC is not enabled
        return null;
    }

    public async componentDidUpdate(prevProps : Readonly<ILiveViewCameraWindowProps>) {
        if (RTCSC_ENABLED) {
            if (this.props.renderCameraMessage !== prevProps.renderCameraMessage) {
                if (this.props.renderCameraMessage) {
                    await this.setActive();
                    // If we have an updated LVC message payload, but there's been no change in RTC client connection
                    // we need to treat it as a new connection
                    if (CameraState.CONNECTED === prevProps.cameraState) {
                        this.setState({
                            firstFrameRendered: false
                        });
                        await this.handleCameraStateChanged(CameraState.CONNECTED);
                    }
                } else {
                    await this.setInactive();
                }
            } else if (this.props.refreshRenderer && this.props.refreshRenderer !== prevProps.refreshRenderer) {
                this.setState({
                    uiState: UIState.LOADING
                });
            }
            if (this.props.cameraState !== prevProps.cameraState) {
                await this.handleCameraStateChanged(this.props.cameraState);
            }
        }
    }

    private uiRendered() {
        this.setState({
            uiState: UIState.RENDERED
        });
    }

    private uiDestroyed() {
        this.setState({
            uiState: UIState.CLEARED
        });
    }

    private videoStarted(event : Event) {
        if (event.type === 'playing') {
            this.setState({
                firstFrameRendered: true
            });
            const message : IBaseOutboundMessage = {
                type: 'cameraFirstFrameRendered'
            };
            this.client.sendMessage(message);
            this.activityToken = this.props.activityTracker.recordActive('LiveViewCamera');
        }
    }

    private videoStopped(event : Event) {
        // If we're not pausing, release focus and reset the video if it had played
        if (!event || event.type !== 'pause') {
            this.releaseFocusAndRecordInactive();
            if (this.videoRef.current && this.state.firstFrameRendered) {
                this.videoRef.current.load();
            }
        }
    }

    private shouldUnmuteVideo() {
        return (this.state.initialSpeakerState === SpeakerState.UNMUTED &&
            this.state.firstFrameRendered &&
            this.state.cameraState === CameraState.CONNECTED &&
            this.state.uiState === UIState.RENDERED &&
            this.focusState === FocusState.FOREGROUND);
    }

    private async setInactive() {
        if (this.mediaClient) {
            this.mediaClient.shutdown();
            this.mediaClientInitialized = false;
        }
        this.videoStopped(undefined);
        this.setState({
            uiState: UIState.CLEARED,
            cameraState: CameraState.DISCONNECTED,
            initialSpeakerState: SpeakerState.DISABLED,
            firstFrameRendered: false
        });
    }

    private acquireFocus() : Promise<void> {
        if (this.liveViewFocusResolver) {
            // Reject existing promise
            this.liveViewFocusResolver.reject();
        }

        return new Promise<void>(((resolve, reject) => {
            this.liveViewFocusResolver = {
                resolve: () => {
                    resolve();
                    this.liveViewFocusResolver = undefined;
                },
                reject: () => {
                    reject();
                    this.liveViewFocusResolver = undefined;
                }
            };
            this.focusToken = this.props.focusManager.acquireFocus(
                AVSInterface.LIVE_VIEW, ChannelName.CONTENT, ContentType.NONMIXABLE, {
                    focusChanged : this.processFocusChanged.bind(this)
                }
            );
        }));
    }

    private releaseFocusAndRecordInactive() {
        this.releaseFocus();
        if (this.activityToken !== undefined) {
            this.props.activityTracker.recordInactive(this.activityToken);
            this.activityToken = undefined;
        }
    }

    private releaseFocus() {
        if (this.focusToken !== undefined) {
            this.props.focusManager.releaseFocus(this.focusToken);
        }
    }

    private processFocusChanged(focusState : FocusState, token : number) {
        if (this.focusToken !== token) {
            // Ignore unexpected token
            return;
        }

        this.focusState = focusState;

        switch (focusState) {
            case FocusState.NONE:
                if (this.videoRef.current && this.state.firstFrameRendered) {
                    this.videoRef.current.pause();
                }
                if (this.liveViewFocusResolver) {
                    this.liveViewFocusResolver.reject();
                }
                this.focusToken = undefined;
                break;
            case FocusState.FOREGROUND:
                if (this.videoRef.current && this.shouldUnmuteVideo()) {
                    this.videoRef.current.muted = false;
                }
                if (this.liveViewFocusResolver) {
                    this.liveViewFocusResolver.resolve();
                }
                break;
            case FocusState.BACKGROUND:
                if (this.videoRef.current) {
                    this.videoRef.current.muted = true;
                }
                if (this.liveViewFocusResolver) {
                    this.liveViewFocusResolver.resolve();
                }
                break;
            default:
                break;
        }
    }
}
