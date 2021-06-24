/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { IImageFilterElement, IBaseFilter } from './ImageFilter';
/**
 * @ignore
 */
export interface IColor extends IBaseFilter {
    color: number;
}
export declare function getColorFilter(filter: Filter): IImageFilterElement;
