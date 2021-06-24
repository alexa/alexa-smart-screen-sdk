/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class AVGText extends AVG {
    private textAnchors;
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    private setInnerHtml();
}
