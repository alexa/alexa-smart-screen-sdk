/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import * as React from 'react';
import './sampleHome.css';
import { IDeviceAppConfig, AudioInputInitiator } from '../lib/config/IDeviceAppConfig';

interface ISampleHomeProps {
    deviceAppConfig? : IDeviceAppConfig;
}

export class SampleHome extends React.Component<ISampleHomeProps, any> {

    public render() {
        let howToTalkString : string;
        let deviceDescriptionString : string = 'Configuring Device...';

        if (this.props.deviceAppConfig) {
            deviceDescriptionString = this.props.deviceAppConfig.description;
            const talkKey = this.props.deviceAppConfig.deviceKeys.talkKey.key.toUpperCase();

            switch (this.props.deviceAppConfig.audioInputInitiator) {
                case AudioInputInitiator.PRESS_AND_HOLD : {
                    howToTalkString = 'Press and Hold \"' + talkKey + '\" then Speak';
                    break;
                }
                case AudioInputInitiator.TAP : {
                    howToTalkString = 'Tap \"' + talkKey + '\" then Speak';
                    break;
                }
                case AudioInputInitiator.WAKEWORD :
                default : {
                    howToTalkString = 'Say \"Alexa\" to Talk';
                    break;
                }
            }
        }

        return (
            <div className='bg'>
                <div className='alexaLogo'></div>
                <div className='usage'>
                    <div className='deviceDescription'>{deviceDescriptionString}</div>
                    <div className='howToTalk'>{howToTalkString}</div>
               </div>
            </div>
        );
    }

    public shouldComponentUpdate(nextProps : ISampleHomeProps) {
        return this.props.deviceAppConfig !== nextProps.deviceAppConfig;
    }
}
