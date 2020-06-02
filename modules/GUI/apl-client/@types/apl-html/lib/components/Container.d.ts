/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { Component, FactoryFunction, IComponentProperties } from './Component';
import APLRenderer from '../APLRenderer';
import { PropertyKey } from '../enums/PropertyKey';
/**
 * @ignore
 */
export interface IContainerProperties extends IComponentProperties {
    [PropertyKey.kPropertyNotifyChildrenChanged]: any;
}
/**
 * @ignore
 */
export declare class Container extends Component {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    setProperties(props: IContainerProperties): Promise<void>;
    protected isLayout(): boolean;
}
