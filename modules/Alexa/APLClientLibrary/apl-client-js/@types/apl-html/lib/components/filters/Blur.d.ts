/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { IImageFilterElement, IBaseFilter } from './ImageFilter';
/**
 * @ignore
 */
export interface IBlur extends IBaseFilter {
    radius: number;
    source: number;
}
export declare function getBlurFilter(filter: Filter, imageSrcArray: string[]): IImageFilterElement | undefined;
