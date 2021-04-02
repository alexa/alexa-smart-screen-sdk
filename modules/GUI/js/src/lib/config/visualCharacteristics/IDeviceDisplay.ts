/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
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
