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

interface ICaptionsViewProps {
    captionsMessage : any;
}

interface ICaptionsViewState {}

export class CaptionsView extends React.Component<ICaptionsViewProps, ICaptionsViewState> {

    constructor(props : ICaptionsViewProps) {
        super(props);

    }

    public render() {
        return (
            <div id='captionsScrim' style={{
                opacity: 1,
                visibility : 'visible'}}>
                <div id='captionsView'>
                    { this.props.captionsMessage ? CaptionsView.renderCaptionLines(this.props.captionsMessage) : ''}
                </div>
            </div>
        );
    }
    private static renderCaptionLines(captionFrame : any) {
        // this is each caption line, and can contain it's own styling
        return captionFrame.captionLines.map((item : any, index : number) => (
            <div key={index} style={{backgroundColor: 'black', padding : '1%'}}>
                {item.text}
            </div>
        ));
    }
    public shouldComponentUpdate(nextProps : ICaptionsViewProps) {
        return this.props.captionsMessage !== nextProps.captionsMessage;
    }
}
