/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction, IGenericPropType } from './Component';
/**
 * @ignore
 */
export declare class ActionableComponent<PropsType = IGenericPropType> extends Component<PropsType> {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    focus: () => void;
    protected blur: () => void;
}
