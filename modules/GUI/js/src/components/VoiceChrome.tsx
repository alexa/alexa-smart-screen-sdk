/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import * as React from 'react';
import './voiceChrome.css';
import {AlexaState} from '../lib/messages/messages';
import {AudioInputInitiator, IDeviceAppConfig} from '../lib/config/IDeviceAppConfig';

type VisibilityProperty = import('csstype').Property.Visibility;

interface IVoiceChromeProps {
    alexaState : AlexaState;
    targetWindowId : string;
    deviceAppConfig : IDeviceAppConfig;
    doNotDisturbSettingEnabled : boolean;
}

interface IVoiceChromeState {
    voiceChromeState : VoiceChromeState;
}

enum VoiceChromeState {
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
    SPEAKING = 'SPEAKING',
    /// VoiceChrome should be in this state at the end of an Alexa Interaction if DoNotDisturb is enabled
    DO_NOT_DISTURB_ENABLED = 'DO NOT DISTURB ENABLED'
}

export class VoiceChrome extends React.Component<IVoiceChromeProps, IVoiceChromeState> {

    protected expectingPromptString : string;
    protected voiceChromeStateAlign : string = this.props.deviceAppConfig.display.shape === 'RECTANGLE'
        ? 'flex-end' : 'center';
    protected lastDoNotDisturbVoiceChromeStateTimeout : number;

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
            voiceChromeState : this.props.alexaState as string as VoiceChromeState
        };
    }

    public render() {
        const chromeOpacity = this.props.alexaState !== undefined  ? 1 : 0;
        const chromeVisibility : VisibilityProperty = chromeOpacity === 0 ? 'hidden' : 'visible';
        return (
            <div id='voiceChromeScrim' style={{
                opacity: chromeOpacity,
                visibility : chromeVisibility}}>
                <div id='voiceChromeState' style={{
                    justifyContent : this.voiceChromeStateAlign
                }}>{this.state.voiceChromeState}</div>
            </div>
        );
    }

    public componentWillReceiveProps(nextProps : IVoiceChromeProps) {
        // Update State if any props have changed
        if (this.props.alexaState !== nextProps.alexaState
            || this.props.doNotDisturbSettingEnabled !== nextProps.doNotDisturbSettingEnabled
            || this.props.targetWindowId !== nextProps.targetWindowId) {

            this.setVoiceChromeStateWithDoNotDisturbIfRequired(nextProps.alexaState, nextProps.targetWindowId,
                nextProps.doNotDisturbSettingEnabled);
        }
    }

    public shouldComponentUpdate(nextProps : IVoiceChromeProps, nextState : IVoiceChromeState) {
        // Update on alexaState change, targetWindow change or VoiceChrome state change
        return this.props.alexaState !== nextProps.alexaState || this.props.targetWindowId !== nextProps.targetWindowId
            || this.props.doNotDisturbSettingEnabled !== nextProps.doNotDisturbSettingEnabled
            || this.state.voiceChromeState !== nextState.voiceChromeState;
    }

    private setVoiceChromeStateWithDoNotDisturbIfRequired(alexaState : AlexaState,
                                                          targetWindowId : string,
                                                          doNotDisturbState : boolean) {

        // Don't show speaking or idle when we have an active visual window
        if ((alexaState === AlexaState.SPEAKING && targetWindowId !== undefined) ||
            (alexaState === AlexaState.IDLE && targetWindowId !== undefined)) {

            alexaState = undefined;
        } else {
            if (alexaState === AlexaState.IDLE) {
                if (doNotDisturbState) {
                    this.setState({
                        voiceChromeState: VoiceChromeState.DO_NOT_DISTURB_ENABLED
                    });
                    clearTimeout(this.lastDoNotDisturbVoiceChromeStateTimeout);
                    this.lastDoNotDisturbVoiceChromeStateTimeout =
                        window.setTimeout(this.setVoiceChromeStateStringWithCurrentAlexaState.bind(this), 5000);
                    return;
                }
            }
        }
        this.setVoiceChromeStateWithAlexaState(alexaState);
    }

    private setVoiceChromeStateWithAlexaState(alexaState : AlexaState) {
        this.setState({
            voiceChromeState: this.getVoiceChromeStateStringWithAlexaState(alexaState) as VoiceChromeState
        });
    }

    private setVoiceChromeStateStringWithCurrentAlexaState() {
        this.setVoiceChromeStateWithAlexaState(this.props.alexaState);
    }

    private getVoiceChromeStateStringWithAlexaState(alexaState : AlexaState) {
        return alexaState === AlexaState.EXPECTING ? this.expectingPromptString : alexaState;
    }
}
