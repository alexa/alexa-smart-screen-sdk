/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { Scrollable } from './Scrollable';
import { PropertyKey } from '../enums/PropertyKey';
/**
 * @ignore
 */
export interface IScrollViewProperties extends IComponentProperties {
    [PropertyKey.kPropertyScrollPosition]: number;
}
/**
 * @ignore
 */
export declare class ScrollView extends Scrollable<IScrollViewProperties> {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    init(): void;
    /**
     * @param component child component
     * @returns [offset,size on direction]
     * @memberof ScrollViewComponent
     */
    getChildTopOffset(component: Component): number;
    setProperties(props: IScrollViewProperties): Promise<void>;
    destroy(): void;
}
