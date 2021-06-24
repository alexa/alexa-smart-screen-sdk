/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
export declare enum CommandType {
    kCommandTypeArray = 0,
    kCommandTypeIdle = 1,
    kCommandTypeSequential = 2,
    kCommandTypeParallel = 3,
    kCommandTypeSendEvent = 4,
    kCommandTypeSetValue = 5,
    kCommandTypeSetState = 6,
    kCommandTypeSpeakItem = 7,
    kCommandTypeSpeakList = 8,
    kCommandTypeScroll = 9,
    kCommandTypeScrollToIndex = 10,
    kCommandTypeScrollToComponent = 11,
    kCommandTypeSelect = 12,
    kCommandTypeSetPage = 13,
    kCommandTypeAutoPage = 14,
    kCommandTypePlayMedia = 15,
    kCommandTypeControlMedia = 16,
    kCommandTypeOpenURL = 17,
    kCommandTypeAnimateItem = 18,
    kCommandTypeSetFocus = 19,
    kCommandTypeClearFocus = 20,
    kCommandTypeFinish = 21,
    kCommandTypeReinflate = 22,
    kCommandTypeCustomEvent = 23
}
