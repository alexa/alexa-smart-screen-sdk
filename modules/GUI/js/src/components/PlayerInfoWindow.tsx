/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import * as React from 'react';
import { IAPLRendererWindowConfig } from '../lib/config/IDeviceAppConfig';
import {FocusManager} from '../lib/focus/FocusManager';
import {ActivityTracker} from '../lib/activity/ActivityTracker';
import {
    IRenderPlayerInfoMessage,
    IRenderStaticDocumentMessage
} from '../lib/messages/messages';
import {
    APLRendererWindow,
    WebsocketConnectionWrapper,
    APLRendererWindowState
} from './APLRendererWindow';
import {
    resolveRenderPlayerInfo
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
}

export class PlayerInfoWindow extends React.Component<IPlayerInfoWindowProps, IPlayerInfoWindowState> {

    protected client : WebsocketConnectionWrapper;
    protected windowConfig : IAPLRendererWindowConfig;
    protected focusManager : FocusManager;
    protected activityTracker : ActivityTracker;
    protected isTargetWindow : boolean;
    protected isRendering : boolean;

    constructor(props : IPlayerInfoWindowProps) {
        super(props);
        this.client = this.props.client;
        this.windowConfig = this.props.windowConfig;
        this.focusManager = this.props.focusManager;
        this.activityTracker = this.props.activityTracker;

        this.state = {
            windowState : APLRendererWindowState.INACTIVE
        };
    }

    private getToggleControls(controls : any) : Map<any, any> {
        if (!controls) {
            return new Map();
        }

        return controls.filter((c : any) => c.type === 'TOGGLE').reduce((m : Map<any, any>, c : any) => {
            m.set(c.name, c);
            return m;
        }, new Map() );
    }

    private toggleControlsChanged(controls : any) : boolean {
        const newToggleControls = this.getToggleControls(controls);
        const oldToggleControls = this.getToggleControls(this.props.playerInfoMessage.payload.controls);
        if (newToggleControls.size !== oldToggleControls.size) {
            // Number of controls has changed
            return true;
        }

        for (const [name, control] of newToggleControls) {
            if (!oldToggleControls.has(name)) {
                return true;
            }

            const oldControl = oldToggleControls.get(name);
            if (control.selected !== oldControl.selected ||
                control.enabled !== oldControl.enabled) {
                return true;
            }
        }
        return false;
    }

    protected handleRenderPlayerInfoMessage(renderPlayerInfoMessage : IRenderPlayerInfoMessage) {
        // If not rendering AND
        // not the current target window,
        // or we have a new playerInfo message or audioItemId,
        // or toggle controls changed - send a new playerInfo renderDocument request
        if (!this.isRendering && (!this.isTargetWindow ||
            !this.props.playerInfoMessage ||
            (renderPlayerInfoMessage.payload.audioItemId !==
                this.props.playerInfoMessage.payload.audioItemId) ||
            this.toggleControlsChanged(renderPlayerInfoMessage.payload.controls))) {
            const renderStaticDocumentMessage : IRenderStaticDocumentMessage =
                resolveRenderPlayerInfo(renderPlayerInfoMessage, RENDER_PLAYER_INFO_WINDOW_ID);
            this.isRendering = true;
            this.client.sendMessage(renderStaticDocumentMessage);

            this.setState({
                windowState : APLRendererWindowState.ACTIVE
            });
        }
    }

    protected onRendererInit() {
        this.isRendering = false;
    }

    protected onRendererDestroyed() {
        this.isRendering = false;
    }

    public render() {
        // Create Player Info APL Window
        const playerInfo = <APLRendererWindow
            id={RENDER_PLAYER_INFO_WINDOW_ID}
            key={RENDER_PLAYER_INFO_WINDOW_ID}
            windowConfig={this.windowConfig}
            windowState={this.state.windowState}
            refreshRenderer={this.props.refreshRenderer}
            client={this.client}
            focusManager={this.focusManager}
            activityTracker={this.activityTracker}
            onRendererInit={this.onRendererInit.bind(this)}
            onRendererDestroyed={this.onRendererDestroyed.bind(this)}
          />;

        return(playerInfo);
    }

    public componentWillReceiveProps(nextProps : IPlayerInfoWindowProps) {
        this.isTargetWindow = nextProps.targetWindowId === RENDER_PLAYER_INFO_WINDOW_ID;
        if (this.props.playerInfoMessage !== nextProps.playerInfoMessage) {
            if (nextProps.playerInfoMessage !== undefined) {
                this.handleRenderPlayerInfoMessage(nextProps.playerInfoMessage);
            } else {
                this.setState({
                    windowState : APLRendererWindowState.INACTIVE
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
}
