/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { IGradient } from '../Image';
import { IImageFilterElement, IBaseFilter } from './ImageFilter';
/**
 * @ignore
 */
export interface IGradientFilter extends IBaseFilter {
    gradient: IGradient;
}
export declare function getGradientFilter(filter: Filter, svgDefsElement: SVGElement, svgUseElement: SVGUseElement): IImageFilterElement;
export declare function getAngleCoords(angle: number): {
    x1: string;
    y1: string;
    x2: string;
    y2: string;
};
