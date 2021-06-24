/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { ILogger } from '../../logging/ILogger';
import { GraphicFilterType } from '../../enums/GraphicFilterType';
export interface IAVGFilterElement {
    filterId: string;
    filterElement: SVGFilterElement;
}
export interface IAVGFilter {
    type: GraphicFilterType;
}
export interface IDropShadowFilter extends IAVGFilter {
    color: number;
    radius: number;
    horizontalOffset: number;
    verticalOffset: number;
}
export declare type AVGFilter = IDropShadowFilter;
export declare function createAndGetFilterElement(filters: AVGFilter[], logger: ILogger): IAVGFilterElement | undefined;
