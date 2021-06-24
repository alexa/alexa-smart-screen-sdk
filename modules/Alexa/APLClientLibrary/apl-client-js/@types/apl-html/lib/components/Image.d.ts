/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { ImageAlign } from '../enums/ImageAlign';
import { ImageScale } from '../enums/ImageScale';
import { PropertyKey } from '../enums/PropertyKey';
import { GradientType } from '../enums/GradientType';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import 'image-scale';
import APLRenderer from '../APLRenderer';
import { ILogger } from '../logging/ILogger';
import { Filter } from '../utils/FilterUtils';
/**
 * @ignore
 */
export interface IGradient {
    angle: number;
    colorRange: number[];
    inputRange: number[];
    type: GradientType;
}
/**
 * @ignore
 */
export interface IImageProperties extends IComponentProperties {
    [PropertyKey.kPropertySource]: string;
    [PropertyKey.kPropertyAlign]: ImageAlign;
    [PropertyKey.kPropertyBorderRadius]: number;
    [PropertyKey.kPropertyBorderWidth]: number;
    [PropertyKey.kPropertyOverlayColor]: number;
    [PropertyKey.kPropertyBorderColor]: number;
    [PropertyKey.kPropertyFilters]: Filter[];
    [PropertyKey.kPropertyOverlayGradient]: IGradient;
    [PropertyKey.kPropertyScale]: ImageScale;
}
/**
 * @ignore
 */
export declare class Image extends Component<IImageProperties> {
    private imgPlaceHolder;
    private $imgPlaceHolder;
    private imageOverlay;
    private $imageOverlay;
    private originalImageElement;
    private $originalImageElement;
    private canvasElement;
    private imageElement;
    private svgDefsElement;
    private svgUseElement;
    private svg;
    private $svg;
    private hasFiltersInCanvas;
    private setSvgImageHrefTimeout;
    private isShadowHolderAdded;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected boundsUpdated(): void;
    private initSvgElement;
    private setBorderRadius;
    protected applyCssShadow: (shadowParams: string) => void;
    private setBorderColor;
    private setBorderWidth;
    private setFilters;
    private setOverlayColor;
    private setOverlayGradient;
    private setSourceAndFilter;
    private getImageSource;
    private setSvgImageHref;
    private applyShadowEffectWhenScaled;
    private hasShadowPropertyDefined;
    /**
     * Check filters.
     * If there is any filter to be implemented in canvas, set hasFiltersInCanvas to true.
     */
    private checkFilters;
    private addSVGFilters(filters, imageSourceArray);
    /**
     * Apply filters one by one based on their order.
     * Skip blur if blur is implemented in CSS.
     */
    private applyFiltersToSvgImageHref;
    static getCssGradient(gradient: IGradient, logger: ILogger): string;
    static getCssPureColorGradient(color: string): string;
    private setImageScale();
    private getImageScale();
    private setImageHolderAlignment();
    private setImageAndSvgAlignment();
}
