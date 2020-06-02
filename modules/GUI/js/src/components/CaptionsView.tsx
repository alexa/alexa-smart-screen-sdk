/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
