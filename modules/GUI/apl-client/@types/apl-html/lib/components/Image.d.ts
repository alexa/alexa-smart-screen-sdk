/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { ImageAlign } from '../enums/ImageAlign';
import { ImageScale } from '../enums/ImageScale';
import { PropertyKey } from '../enums/PropertyKey';
import { FilterType } from '../enums/FilterType';
import { GradientType } from '../enums/GradientType';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import 'image-scale';
import APLRenderer from '../APLRenderer';
import { NoiseFilterKind } from '../enums/NoiseFilterKind';
import { ILogger } from '../logging/ILogger';
/**
 * @ignore
 */
export interface IBaseFilter {
    type: FilterType;
}
/**
 * @ignore
 */
export interface IBlur extends IBaseFilter {
    radius: number;
}
/**
 * @ignore
 */
export interface INoise extends IBaseFilter {
    kind?: NoiseFilterKind;
    useColor?: boolean;
    sigma?: number;
}
/**
 * @ignore
 */
export declare type Filter = IBlur | INoise;
/**
 * Type guard for IBlur
 * @param filter
 * @ignore
 */
export declare function isIBlur(filter: Filter): filter is IBlur;
/**
 * Type guard for INoise
 * @param filter
 * @ignore
 */
export declare function isINoise(filter: Filter): filter is INoise;
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
 * TODO: Figure out why two Images constructed with only one Image in APL document
 */
export declare class Image extends Component<IImageProperties> {
    private imgPlaceHolder;
    private $imgPlaceHolder;
    private imageOverlay;
    private $imageOverlay;
    private imageElement;
    private $imageElement;
    private canvasElement;
    private $canvasElement;
    private allFiltersInCanvas;
    private hasFiltersInCanvas;
    private applyFiltersTimeout;
    private isShadowHolderAdded;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected boundsUpdated(): void;
    private setBorderRadius;
    protected applyCssShadow: (shadowParams: string) => void;
    private setBorderColor;
    private setBorderWidth;
    private setFilters;
    private setOverlayColor;
    private setOverlayGradient;
    private setSource;
    private applyShadowEffectWhenScaled;
    private hasShadowPropertyDefined;
    /**
     * Check filters, update hasFiltersInCanvas and allFiltersInCanvas.
     *
     * If there is any blur filter applied after noise, then need to apply
     * both noise and blur filters in canvas, set allFiltersInCanvas to true
     * otherwise implement only noise in canvas, set allFiltersInCanvas to false.
     *
     * If there is any filter to be implemented in canvas, set hasFiltersInCanvas to true.
     */
    private checkFilters;
    /**
     * Apply filters one by one based on their order.
     * Skip blur if blur is implemented in CSS.
     */
    private applyFiltersOnCanvas;
    private convertFilterComponentToCss(filter);
    private getCssFilterFromApl(filters);
    static getCssGradient(gradient: IGradient, logger: ILogger): string;
    static getCssPureColorGradient(color: string): string;
    private setImageScale();
    private getImageScale();
    private setImageHolderAlignment();
    private setImageAndCanvasAlignment();
}
