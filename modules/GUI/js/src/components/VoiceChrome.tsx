/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import * as React from 'react';
import './voiceChrome.css';
import { AlexaState } from '../lib/messages/messages';
import { VisibilityProperty } from 'csstype';
import { IDeviceAppConfig, AudioInputInitiator } from '../lib/config/IDeviceAppConfig';

interface IVoiceChromeProps {
    alexaState : AlexaState;
    targetWindowId : string;
    deviceAppConfig : IDeviceAppConfig;
}

interface IVoiceChromeState {
    alexaState : AlexaState;
}

export class VoiceChrome extends React.Component<IVoiceChromeProps, IVoiceChromeState> {

    protected voiceChromeStateString : string;
    protected expectingPromptString : string;
    protected voiceChromeStateAlign : string = this.props.deviceAppConfig.display.shape === 'RECTANGLE'
        ? 'flex-end' : 'center';

    constructor(props : IVoiceChromeProps) {
        super(props);

        const talkKey = this.props.deviceAppConfig.deviceKeys.talkKey.key.toUpperCase();

        switch (this.props.deviceAppConfig.audioInputInitiator) {
            case AudioInputInitiator.PRESS_AND_HOLD : {
                this.expectingPromptString = 'Press and Hold \"' + talkKey + '\" then Speak.';
                break;
            }
            default : {
                break;
            }
        }

        this.state = {
            alexaState : this.props.alexaState
        };
    }

    public render() {
        const chromeOpacity = this.state.alexaState !== undefined  ? 1 : 0;
        const chromeVisibility : VisibilityProperty = chromeOpacity === 0 ? 'hidden' : 'visible';

        if (this.state.alexaState !== undefined) {
            this.voiceChromeStateString = this.state.alexaState === AlexaState.EXPECTING ? this.expectingPromptString :
                this.state.alexaState;
        }

        return (
            <div id='voiceChromeScrim' style={{
                opacity: chromeOpacity,
                visibility : chromeVisibility}}>
                <div id='voiceChromeState' style={{
                    justifyContent : this.voiceChromeStateAlign
                }}>{this.voiceChromeStateString}</div>
            </div>
        );
    }

    public componentWillReceiveProps(nextProps : IVoiceChromeProps) {
        let alexaState : AlexaState = nextProps.alexaState;
        // Don't show speaking or idle when we have an active visual window
        if ((alexaState === AlexaState.SPEAKING
            && (this.props.targetWindowId !== undefined || nextProps.targetWindowId !== undefined)) ||
            (alexaState === AlexaState.IDLE && nextProps.targetWindowId !== undefined)) {
            alexaState = undefined;
        }

        if (alexaState !== this.state.alexaState) {
            this.setState({
                alexaState
            });
        }
    }

    public shouldComponentUpdate(nextProps : IVoiceChromeProps, nextState : IVoiceChromeState) {
        // Only update on alexaState change or targetWindow change
        return this.state.alexaState !== nextState.alexaState || this.props.targetWindowId !== nextProps.targetWindowId;
    }
}
