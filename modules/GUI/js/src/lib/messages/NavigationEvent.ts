/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

/**
 * Copy of alexaClientSDK/smartScreenSDKInterfaces/NavgationEvent
 *
 * @enum
 * @exports
 */
export enum NavigationEvent {
    /**
     * Exit screen.
     */
    EXIT = 'EXIT',

    /**
     * Go back on screen.
     */
    BACK = 'BACK',

    /**
     * Guard option for unknown event.
     */
    UNKNOWN = 'UNKNOWN'
}
