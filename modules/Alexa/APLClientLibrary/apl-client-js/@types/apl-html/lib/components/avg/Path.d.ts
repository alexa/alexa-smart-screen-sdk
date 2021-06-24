/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class Path extends AVG {
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    private setPathLength();
}
