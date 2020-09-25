/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { ActionableComponent } from './ActionableComponent';
import APLRenderer from '../APLRenderer';
/**
 * Touchable components are components that can receive input from touch
 * or pointer events invoke handlers to support custom touch interaction behavior.
 */
export declare abstract class TouchableComponent<TouchableProps = IComponentProperties> extends ActionableComponent<TouchableProps> {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
}
