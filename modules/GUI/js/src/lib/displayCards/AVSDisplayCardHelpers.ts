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

import {
    IRenderTemplateMessage,
    IRenderPlayerInfoMessage,
    IExecuteCommandsMessage,
    IRenderStaticDocumentMessage,
    createRenderStaticDocumentMessage,
    createExecuteCommandsMessage
} from '../messages/messages';

import * as AVSDisplayCards from './AVSDisplayCards.json';
import * as AVSDisplayCardSupportedViewports from './AVSDisplayCardSupportedViewports.json';

const RENDER_PLAYER_INFO_KEY : string = 'RenderPlayerInfo';

/**
 * These helper functions handle presentation of TemplateRuntime 'DisplayCard' directives
 * by binding RenderTemplate and RenderPlayerInfo payloads to APL templates that render the
 * visual metadata.
 *
 * The local AVSDisplayCards APL document references a hosted package that manages these templates,
 * and the AVSDisplayCardSupportedViewports defines the collection of supported viewports.
 *
 * For more information on DisplayCards:
 * https://developer.amazon.com/docs/alexa-voice-service/display-cards-overview.html
 */

/**
 * Resolves a RenderTemplate message to a local APL RenderDocument message.
 * The RenderTemplate payload is used as datasource for APL document.
 *
 * @param message IRenderTemplateMessage containing RenderTemplate payload.
 * @param windowId ID of the window this message will target with an APL render response.
 */
export const resolveRenderTemplate =
    (message : IRenderTemplateMessage, windowId : string) : IRenderStaticDocumentMessage => {
    return createRenderStaticDocumentMessage(
        message.token,
        windowId,
        AVSDisplayCards,
        message.payload,
        AVSDisplayCardSupportedViewports);
};

/**
 * Resolves a RenderPlayerInfo message to a local APL RenderDocument message.
 * The RenderPlayerInfo payload is used as datasource for APL document.
 *
 * @param message IRenderPlayerInfoMessage containing RenderPlayerInfo payload.
 * @param windowId ID of the window this message will target with an APL render response.
 */
export const resolveRenderPlayerInfo =
    (message : IRenderPlayerInfoMessage, windowId : string) : IRenderStaticDocumentMessage => {
    const payload = message.payload;
    payload.type = RENDER_PLAYER_INFO_KEY;
    payload.audioPlayerInfo = {
        state: message.audioPlayerState,
        mediaOffsetInMilliseconds: message.audioOffset
    };
    return createRenderStaticDocumentMessage(
        message.token,
        windowId,
        AVSDisplayCards,
        payload,
        AVSDisplayCardSupportedViewports);
};

/**
 * Resolves a RenderPlayerInfo message to a local APL ExecuteCommands message.
 * This is used to udpate an active RenderPlayerInfo document.
 *
 * @param message IRenderPlayerInfoMessage containing RenderPlayerInfo payload.
 */
export const resolveRenderPlayerInfoCommand = (
        message : IRenderPlayerInfoMessage) : IExecuteCommandsMessage => {
    const command = {
        type : 'updateRenderPlayerInfo',
        mediaOffsetInMilliseconds: message.audioOffset,
        mediaLengthInMilliseconds : message.payload.content.mediaLengthInMilliseconds,
        audioPlayerState: message.audioPlayerState,
        controls: message.payload.controls
    };
    return createExecuteCommandsMessage(RENDER_PLAYER_INFO_KEY, command);
};
