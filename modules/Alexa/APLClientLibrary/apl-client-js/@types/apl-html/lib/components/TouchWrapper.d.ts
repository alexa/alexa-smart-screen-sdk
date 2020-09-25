/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { TouchableComponent } from './TouchableComponent';
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
export declare class TouchWrapper extends TouchableComponent<ITouchWrapperProperties> {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    private updateUponChildrenChange;
}
