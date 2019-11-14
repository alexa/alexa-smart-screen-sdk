/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

/**
 * Copy from AVSCommon/SDKInterfaces/FocusManagerInterface
 * @see https://developer.amazon.com/docs/alexa-voice-service/focus-management.html
 *
 * @enum
 * @exports
 */
export enum ChannelName {
    /// Dialog Channel name.
    DIALOG = 'Dialog',

    /// Alerts Channel name.
    ALERTS = 'Alerts',

    /// Communications Channel name.
    COMMUNICATIONS = 'Communications',

    /// Content Channel name.
    CONTENT = 'Content',

    /// Visual Channel name.
    VISUAL = 'Visual'
}
