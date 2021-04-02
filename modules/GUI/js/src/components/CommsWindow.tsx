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

// Stub implementation

import * as React from 'react';
import './commsWindow.css';

import {IClient} from '../lib/messages/client';
import {ICallStateChangeMessage} from '../lib/messages/messages';

export const RENDER_COMMS_WINDOW_ID = 'renderCommsWindow';

interface ICommsWindowProps {
        callStateInfo : ICallStateChangeMessage;
        client : IClient;
}

export class CommsWindow extends React.Component<ICommsWindowProps> {
    public render() : React.ReactNode {
        return null;
    }
}
