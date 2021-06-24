/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { IBaseFilter, IImageFilterElement } from './ImageFilter';
/**
 * @ignore
 */
export interface ISaturate extends IBaseFilter {
    amount: number;
    source?: number;
}
export declare function getSaturateFilter(filter: Filter, imageSrcArray: string[]): IImageFilterElement | undefined;
