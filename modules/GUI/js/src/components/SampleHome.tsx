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
