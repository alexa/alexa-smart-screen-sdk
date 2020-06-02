/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { ViewportShape } from 'apl-client';

export type DisplayType =
    'PIXEL';

export type DisplayTouchSupport =
    'UNSUPPORTED'
    | 'SINGLE';

export type DisplayDimensionsUnit =
    'PIXEL'
    | 'DP';

export type DisplayPhysicalSizeUnit =
    'CENTIMETER'
    | 'INCHES';

export type DisplayPixelDensity =
    120
    | 160
    | 213
    | 240
    | 320
    | 480
    | 640;

export interface IDisplayDimensionValues {
    width : number;
    height : number;
}

export interface IDisplayDimensions {
    physicalSize? : {
        unit : DisplayPhysicalSizeUnit;
        value : IDisplayDimensionValues;
    };
}

export interface IDisplayPixelDimensions extends IDisplayDimensions {
    resolution : {
        unit : 'PIXEL';
        value : IDisplayDimensionValues;
    };
    pixelDensity : {
        unit : 'DPI';
        value : DisplayPixelDensity;
    };
    densityIndependentResolution? : {
        unit : 'DP';
        value : IDisplayDimensionValues;
    };
}

export interface IDeviceDisplay {
    type : DisplayType;
    touch : DisplayTouchSupport[];
    shape : ViewportShape;
    dimensions : IDisplayDimensions;
 }
