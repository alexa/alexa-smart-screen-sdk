/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
