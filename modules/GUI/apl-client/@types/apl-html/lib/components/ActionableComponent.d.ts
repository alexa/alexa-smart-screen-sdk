/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction, IGenericPropType } from './Component';
/**
 * @ignore
 */
export declare class ActionableComponent<PropsType = IGenericPropType> extends Component<PropsType> {
    private focused;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected focus: () => void;
    protected blur: () => void;
}
