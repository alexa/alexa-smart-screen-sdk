/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

/**
 * Copy of AVSCommon/AVS/FocusState
 *
 * @enum
 * @exports
 */
export enum FocusState {
    /// Represents the highest focus a Channel can have.
    FOREGROUND = 'FOREGROUND',

    /// Represents the intermediate level focus a Channel can have.
    BACKGROUND = 'BACKGROUND',

    /// This focus is used to represent when a Channel is not being used or when an observer should stop.
    NONE = 'NONE'
}
