/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../APLRenderer';
import { PropertyKey } from '../enums/PropertyKey';
import { ScrollDirection } from '../enums/ScrollDirection';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { Scrollable } from './Scrollable';
/**
 * @ignore
 */
export interface IMultiChildScrollableProperties extends IComponentProperties {
    [PropertyKey.kPropertyScrollDirection]: ScrollDirection;
    [PropertyKey.kPropertyScrollPosition]: number;
    [PropertyKey.kPropertyNotifyChildrenChanged]: any;
}
export interface IItem {
    index: number;
    component: Component;
    prev?: IItem;
    next?: IItem;
}
/**
 * @ignore
 */
export declare abstract class MultiChildScrollable extends Scrollable<IMultiChildScrollableProperties> {
    private childCount;
    private first;
    private last;
    private childCache;
    protected fullyLoaded: boolean;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    init(): void;
    setProperties(props: IMultiChildScrollableProperties): Promise<void>;
    destroy(): void;
    protected allowFocus(requestedDistance: number, moveTo: HTMLDivElement): boolean;
    private adjustIndices(target, insert);
    private updateUponChildrenChange;
    private setScrollDirection;
    private createItem(index, insertAt?);
    private onUpdate;
}
