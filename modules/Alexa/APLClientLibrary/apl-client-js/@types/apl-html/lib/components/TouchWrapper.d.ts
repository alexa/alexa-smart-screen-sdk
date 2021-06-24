/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { ActionableComponent } from './ActionableComponent';
import { PropertyKey } from '../enums/PropertyKey';
/**
 * @ignore
 */
export interface ITouchWrapperProperties extends IComponentProperties {
    [PropertyKey.kPropertyNotifyChildrenChanged]: any;
}
/**
 * @ignore
 */
export declare class TouchWrapper extends ActionableComponent<ITouchWrapperProperties> {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    private updateUponChildrenChange;
}
