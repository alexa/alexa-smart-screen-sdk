/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { ILogger } from '../../logging/ILogger';
import { GradientType } from '../../enums/GradientType';
import { GradientSpreadMethod } from '../../enums/GradientSpreadMethod';
import { GradientUnits } from '../../enums/GradientUnits';
/**
 * @ignore
 */
export interface IAVGGradient {
    type: GradientType;
    colorRange: number[];
    inputRange: number[];
    spreadMethod: GradientSpreadMethod;
    units: GradientUnits;
    x1: number;
    x2: number;
    y1: number;
    y2: number;
    centerX: number;
    centerY: number;
    radius: number;
}
export declare function getGradientElementId(gradient: IAVGGradient, transform: string, parent: Element, logger: ILogger): string;
