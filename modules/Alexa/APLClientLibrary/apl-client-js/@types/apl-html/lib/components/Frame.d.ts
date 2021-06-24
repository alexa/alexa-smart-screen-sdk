/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../APLRenderer';
import { PropertyKey } from '../enums/PropertyKey';
import { Component, FactoryFunction, IComponentProperties } from './Component';
/**
 * @ignore
 */
export interface IFrameProperties extends IComponentProperties {
    [PropertyKey.kPropertyBackgroundColor]: number;
    [PropertyKey.kPropertyBorderRadii]: APL.Radii;
    [PropertyKey.kPropertyBorderColor]: number;
    [PropertyKey.kPropertyBorderWidth]: number;
}
/**
 * @ignore
 */
export declare class Frame extends Component<IFrameProperties> {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    private setBackgroundColor;
    private setBorderRadii;
    private setBorderColor;
    private setBorderWidth;
}
