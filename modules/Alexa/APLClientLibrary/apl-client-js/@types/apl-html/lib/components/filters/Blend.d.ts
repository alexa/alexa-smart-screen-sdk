/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { BlendMode } from '../../enums/BlendMode';
import { IBaseFilter, IImageFilterElement } from './ImageFilter';
/**
 * @ignore
 */
export interface IBlend extends IBaseFilter {
    mode: BlendMode;
    source: number;
    destination: number;
}
export declare function getBlendFilter(filter: Filter, imageSrcArray: string[]): IImageFilterElement | undefined;
/**
 * Return Blend Mode
 * https://codepen.io/yoksel/pen/BiExv
 */
export declare function getBlendMode(mode: BlendMode): string;
