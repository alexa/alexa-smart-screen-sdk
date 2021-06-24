/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { NoiseFilterKind } from '../../enums/NoiseFilterKind';
import { Filter } from '../../utils/FilterUtils';
import { IBaseFilter } from './ImageFilter';
/**
 * @ignore
 */
export interface INoise extends IBaseFilter {
    kind?: NoiseFilterKind;
    useColor?: boolean;
    sigma?: number;
}
/**
 * Type guard for INoise
 * @param filter
 * @ignore
 */
export declare function isINoise(filter: Filter): filter is INoise;
export declare class Noise {
    private static readonly DEFAULT_USE_COLOR;
    private static readonly DEFAULT_KIND;
    private static readonly DEFAULT_SIGMGA;
    private static readonly RANDOM_SEED;
    private useColor;
    private kind;
    private sigma;
    private generate;
    private z1;
    constructor(useColor?: boolean, kind?: NoiseFilterKind, sigma?: number);
    private gaussianNoise;
    private uniformNoise;
    /**
     * Add noise to provided one-dimensional image data.
     * @param imageData
     */
    addNoise(imageData: ImageData): ImageData;
}
