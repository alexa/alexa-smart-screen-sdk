/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class Group extends AVG {
    children: AVG[];
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    private setClipPath();
    bootStrapChildren(graphic: APL.GraphicElement, logger: ILogger): void;
}
