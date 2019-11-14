/**
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { Component, FactoryFunction } from './Component';
import APLRenderer from '../APLRenderer';
/**
 * @ignore
 */
export declare class Container extends Component {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
}
