/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

import { FocusState } from './FocusState';

/**
 * Interface to be implemented in order to track focusChanged events.
 *
 * @interface
 * @exports
 */
export interface IChannelObserver {
    focusChanged(focusState : FocusState, token : number) : void;
}
