/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class Patterns {
    protected graphicPattern: APL.GraphicPattern;
    protected transform: string;
    protected parent: Element;
    protected logger: ILogger;
    avgElements: AVG[];
    constructor(graphicPattern: APL.GraphicPattern, transform: string, parent: Element, logger: ILogger);
    getPatternId(): string;
}
