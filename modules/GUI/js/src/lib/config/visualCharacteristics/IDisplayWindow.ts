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
export type WindowType =
    'STANDARD'
    | 'OVERLAY';

export type WindowPreferredUseageType =
    'PRIMARY';

export type WindowConfigurationType =
    'DISCRETE'
    | 'CONTINUOUS';

export type WindowMeasurementUnit =
    'PIXEL';

export interface IDisplayWindowSizeDimensionValues {
    width : number;
    height : number;
}

export interface IDisplayWindowSizeDimensions {
    unit : WindowMeasurementUnit;
    value : IDisplayWindowSizeDimensionValues;
}

export interface IDisplayWindowSize {
    id : string;
    type : WindowConfigurationType;
}

export interface IDisplayWindowDiscreteSize extends IDisplayWindowSize {
    value : IDisplayWindowSizeDimensions;
}

export interface IDisplayWindowConfiguration {
    sizes : IDisplayWindowSize[];
    interactionModes : string[];
}

export interface IDisplayWindowTemplate {
    id : string;
    type : WindowType;
    preferredUsage : WindowPreferredUseageType;
    configuration : IDisplayWindowConfiguration;
}
