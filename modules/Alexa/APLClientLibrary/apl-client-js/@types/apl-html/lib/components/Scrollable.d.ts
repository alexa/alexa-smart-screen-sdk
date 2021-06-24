/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../APLRenderer';
import { ScrollDirection } from '../enums/ScrollDirection';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { ActionableComponent } from './ActionableComponent';
/**
 * @ignore
 */
export declare type ScrollHandler = (pos: number) => void;
/**
 * @ignore
 */
export interface IScollOptions {
    suppressScrollX?: boolean;
    suppressScrollY?: boolean;
    useBothWheelAxes?: boolean;
    scrollXMarginOffset?: number;
    scrollYMarginOffset?: number;
    handlers?: string[];
}
/**
 * @ignore
 */
export declare abstract class Scrollable<ScrollableProps = IComponentProperties> extends ActionableComponent<ScrollableProps> {
    static FOCUS_SCROLL_VELOCITY: number;
    direction: ScrollDirection;
    protected scrollbar: any;
    protected length: 'width' | 'height';
    protected scrollSize: 'scrollHeight' | 'scrollWidth';
    protected scrollSide: 'scrollTop' | 'scrollLeft';
    protected side: 'left' | 'top';
    protected hasFocusableChildren: boolean;
    protected startGap: HTMLDivElement;
    protected $startGap: JQuery<HTMLDivElement>;
    protected endGap: HTMLDivElement;
    protected $endGap: JQuery<HTMLDivElement>;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    protected allowFocus(requestedDistance: number, moveTo: HTMLDivElement): boolean;
    getScrollPosition(): number;
    getScrollLength(): number;
    getPageSize(): number;
    getDirection(): ScrollDirection;
}
