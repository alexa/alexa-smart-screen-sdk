/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { IImageFilterElement, IBaseFilter } from './ImageFilter';
/**
 * @ignore
 */
export interface IGrayscale extends IBaseFilter {
    amount: number;
    source: number;
}
export declare function getGrayscaleFilter(filter: Filter, imageSrcArray: string[]): IImageFilterElement | undefined;
/**
 * Return grayscale matrix
 * Reference https://www.w3.org/TR/filter-effects-1/#grayscaleEquivalent
 * @return {string} grayscale matrix
 */
export declare function getGrayscaleMatrix(amount: number): string;
