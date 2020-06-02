/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction } from './Component';
import { ActionableComponent } from './ActionableComponent';
/**
 * @ignore
 */
export declare class TouchWrapper extends ActionableComponent {
    private pressedDown;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    private onMouseDown;
    private onMouseLeave;
    private onMouseUp;
    private keydown;
    private keyup;
}
