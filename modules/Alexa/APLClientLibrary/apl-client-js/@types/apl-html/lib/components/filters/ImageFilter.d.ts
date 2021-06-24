/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Filter } from '../../utils/FilterUtils';
import { FilterType } from '../../enums/FilterType';
/**
 * @ignore
 */
export interface IBaseFilter {
    type: FilterType;
}
/**
 * IImageFilterElement contains:
 * @param filterId component id assigned by core, eg: 1009. Use it to set as the unique filter id
 * @param filterElement svg filter element. feGaussianBlur/feBlend etc ...
 * @param filterImageArray feImage array, store all the SVG filter primitive,
 * fetches image data from an external source and provides the pixel data as output
 */
export interface IImageFilterElement {
    filterId: string;
    filterElement: SVGElement;
    filterImageArray: SVGFEImageElement[];
}
/**
 * Check if the image url match standard bitmap format or from http/s resource
 */
export declare const BITMAP_IMAGE_REGEX_CHECK: string;
export declare class ImageFilter {
    private imageArray;
    private filters;
    private svgFilter;
    private svgDefsElement;
    private logger;
    private svgUseElement;
    /**
     * ImageFilter Constructor
     * @param filters filters get from kPropertyFilters
     * @param imageSrcArray image urls get from kPropertySource
     */
    constructor(filters: Filter[], imageSrcArray: string[], svgDefsElement: SVGElement, svgUseElement: SVGUseElement);
    private applyFilters();
    /**
     * Append SVG filter primitive
     * Order is important: need append filterImageArray first.
     */
    private appendFilterElement(filterElement);
    /**
     * Return SVG Filter Element
     * @return {SVGFilterElement}
     */
    getSvgFilterElement(): SVGFilterElement | undefined;
}
