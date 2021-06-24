/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { GraphicData } from "./avg/APLGraphic";
import { GraphicPatternData } from "./avg/APLGraphicPattern";
import { Filter } from 'apl-html';
export declare function toRect(value: [number, number, number, number]): APL.Rect | undefined;
export declare function toTransform(value: [number, number, number, number, number, number]): string;
export declare function toColor(value: any): number;
export declare function toStyledText(value: {
    text: string;
    spans: Array<[number, number, number]>;
}): APL.StyledText | string;
export declare function toGraphic(value: GraphicData): APL.Graphic | undefined;
export declare function toGraphicPattern(value: GraphicPatternData): APL.GraphicPattern | undefined;
export declare function toRadii(value: [number, number, number, number]): APL.Radii;
export declare function toDimension(value: number): number;
export declare function toFilters(value: Filter[]): Filter[];
export declare function toGradient(value: {
    type: number;
    colorRange: string[];
    inputRange: number[];
    angle: number;
    spreadMethod: number;
    x1: number;
    y1: number;
    x2: number;
    y2: number;
    centerX: number;
    centerY: number;
    radius: number;
    units: number;
}): object;
