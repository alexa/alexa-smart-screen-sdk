/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import * as React from 'react';
import { IAPLRendererWindowConfig } from '../lib/config/IDeviceAppConfig';
import {FocusManager} from '../lib/focus/FocusManager';
import {ActivityTracker} from '../lib/activity/ActivityTracker';
import {
    IExecuteCommandsMessage,
    IRenderPlayerInfoMessage,
    IRenderStaticDocumentMessage
} from '../lib/messages/messages';
import {
    APLRendererWindow,
    WebsocketConnectionWrapper,
    APLRendererWindowState
} from './APLRendererWindow';
import {
    resolveRenderPlayerInfo,
    resolveRenderPlayerInfoCommand
} from '../lib/displayCards/AVSDisplayCardHelpers';

export const RENDER_PLAYER_INFO_WINDOW_ID = 'renderPlayerInfo';

interface IPlayerInfoWindowProps {
    playerInfoMessage : IRenderPlayerInfoMessage;
    targetWindowId : string;
    refreshRenderer : boolean;
    windowConfig : IAPLRendererWindowConfig;
    client : WebsocketConnectionWrapper;
    focusManager : FocusManager;
    activityTracker : ActivityTracker;
}

interface IPlayerInfoWindowState {
    windowState : APLRendererWindowState;
    onRendererInit() : void;
    onRendererDestroyed() : void;
}

export class PlayerInfoWindow extends React.Component<IPlayerInfoWindowProps, IPlayerInfoWindowState> {

    protected client : WebsocketConnectionWrapper;
    protected windowConfig : IAPLRendererWindowConfig;
    protected focusManager : FocusManager;
    protected activityTracker : ActivityTracker;
    protected isTargetWindow : boolean;

    protected renderTimerIntervalId : number;
    protected renderTimerOffset : number;
    protected renderTimeinMs : number;

    constructor(props : IPlayerInfoWindowProps) {
        super(props);
        this.client = this.props.client;
        this.windowConfig = this.props.windowConfig;
        this.focusManager = this.props.focusManager;
        this.activityTracker = this.props.activityTracker;

        this.state = {
            windowState : APLRendererWindowState.INACTIVE,
            onRendererInit : undefined,
            onRendererDestroyed : undefined
        };
    }

    protected startRenderTimer() {
        if (!this.renderTimerIntervalId) {
            this.renderTimerOffset = Date.now();
            this.renderTimerIntervalId = window.setInterval(() => {
                this.renderTimeinMs = Date.now() - this.renderTimerOffset;
            }, 1);
        }
    }

    protected stopRenderTimer() {
        if (this.renderTimerIntervalId) {
            clearInterval(this.renderTimerIntervalId);
            this.renderTimerIntervalId = null;
            this.renderTimeinMs = 0;
        }
    }

    protected handleRenderPlayerInfoMessage(renderPlayerInfoMessage : IRenderPlayerInfoMessage) {

        const executeCommandsMessage : IExecuteCommandsMessage =
                resolveRenderPlayerInfoCommand(renderPlayerInfoMessage);

        // If not waiting to render AND
        // this is not the current target window,
        // or we have a new playerInfo audioItemId - send a new playerInfo renderDocument request
        if (!this.renderTimerIntervalId && (!this.isTargetWindow ||
                !this.props.playerInfoMessage ||
                (renderPlayerInfoMessage.payload.audioItemId !==
                this.props.playerInfoMessage.payload.audioItemId))) {
            const renderStaticDocumentMessage : IRenderStaticDocumentMessage =
                resolveRenderPlayerInfo(renderPlayerInfoMessage, RENDER_PLAYER_INFO_WINDOW_ID);

            this.client.sendMessage(renderStaticDocumentMessage);
            // Start render timer
            this.startRenderTimer();

            this.setState({
                windowState : APLRendererWindowState.ACTIVE,
                onRendererInit: () => {
                    // Add render time to audioOffset for accurate progress bar
                    renderPlayerInfoMessage.audioOffset = renderPlayerInfoMessage.audioOffset + this.renderTimeinMs;
                    this.stopRenderTimer();
                    const offsetExecuteCommandsMessage : IExecuteCommandsMessage =
                        resolveRenderPlayerInfoCommand(renderPlayerInfoMessage);
                    if (offsetExecuteCommandsMessage) {
                        this.client.sendMessage(offsetExecuteCommandsMessage);
                    }
                },
                onRendererDestroyed: () => {
                    this.stopRenderTimer();
                }
            });

        } else if (executeCommandsMessage) {
            // Update active render player info card via command
            this.client.sendMessage(executeCommandsMessage);
            // Make sure the window is active
            if (this.state.windowState === APLRendererWindowState.INACTIVE) {
                this.setState({
                    windowState : APLRendererWindowState.ACTIVE
                });
            }
        }
    }

    public render() {
        // Create Player Info APL Window
        const playerInfo = <APLRendererWindow
            id={RENDER_PLAYER_INFO_WINDOW_ID}
            key={RENDER_PLAYER_INFO_WINDOW_ID}
            windowConfig={this.windowConfig}
            windowState={this.state.windowState}
            onRendererInit={this.state.onRendererInit}
            onRendererDestroyed={this.state.onRendererDestroyed}
            refreshRenderer={this.props.refreshRenderer}
            client={this.client}
            focusManager={this.focusManager}
            activityTracker={this.activityTracker}
          />;

        return(playerInfo);
    }

    public componentWillReceiveProps(nextProps : IPlayerInfoWindowProps) {
        this.isTargetWindow = nextProps.targetWindowId === RENDER_PLAYER_INFO_WINDOW_ID;
        if (this.props.playerInfoMessage !== nextProps.playerInfoMessage) {
            if (nextProps.playerInfoMessage !== undefined) {
                this.handleRenderPlayerInfoMessage(nextProps.playerInfoMessage);
            } else {
                // Clear the window on an empty message
                this.stopRenderTimer();
                this.setState({
                    windowState : APLRendererWindowState.INACTIVE,
                    onRendererInit : undefined,
                    onRendererDestroyed : undefined
                });
            }
        } else if (nextProps.targetWindowId === undefined &&
            // If the target window is cleared, but the PlayerInfo window
            // is still ACTIVE and STOPPED, we need to re-render
            // to prevent a stale card from showing.
            this.state.windowState === APLRendererWindowState.ACTIVE
            && this.props.playerInfoMessage !== undefined
            && this.props.playerInfoMessage.audioPlayerState === 'STOPPED') {
            this.handleRenderPlayerInfoMessage(this.props.playerInfoMessage);
        }
    }

    public componentWillUnmount() {
        this.stopRenderTimer();
    }
}
