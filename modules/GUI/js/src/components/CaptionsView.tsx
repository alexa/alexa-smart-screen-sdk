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
import './captionsView.css';
import {LocaleManager} from '../lib/utils/localeManager';

type DirectionProperty = import('csstype').Property.Direction;
type TextAlignProperty = import('csstype').Property.TextAlign;

interface ICaptionsViewProps {
    captionsMessage : any;
}

interface ICaptionsViewState {}

export class CaptionsView extends React.Component<ICaptionsViewProps, ICaptionsViewState> {

    constructor(props : ICaptionsViewProps) {
        super(props);
    }

    public render() {
        let textAlign : TextAlignProperty = 'left';
        let direction : DirectionProperty = 'ltr';
        switch (LocaleManager.getLocaleLayoutDirection()) {
            case 'RTL':
                textAlign = 'right';
                direction = 'rtl';
                break;
            default:
                break;
        }
        return (
            <div id='captionsScrim' style={{
                opacity: 1,
                visibility : 'visible'}}>
                <div id='captionsView' style={{textAlign}}>
                    { this.props.captionsMessage ? CaptionsView.renderCaptionLines(
                        this.props.captionsMessage, direction) : ''}
                </div>
            </div>
        );
    }
    private static renderCaptionLines(captionFrame : any, direction : DirectionProperty) {
        // this is each caption line, and can contain it's own styling
        return captionFrame.captionLines.map((item : any, index : number) => (
            <p key={index} style={{direction, backgroundColor: 'black', padding : '1%'}}>
                {item.text}
            </p>
        ));
    }
    public shouldComponentUpdate(nextProps : ICaptionsViewProps) {
        return this.props.captionsMessage !== nextProps.captionsMessage;
    }
}
