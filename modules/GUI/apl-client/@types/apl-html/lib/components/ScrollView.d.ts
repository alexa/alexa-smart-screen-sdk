import APLRenderer from '../APLRenderer';
import { Component, FactoryFunction } from './Component';
import { Scrollable } from './Scrollable';
/**
 * @ignore
 */
export declare class ScrollView extends Scrollable {
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    init(): void;
    /**
     * @param component child component
     * @returns [offset,size on direction]
     * @memberof ScrollViewComponent
     */
    getChildTopOffset(component: Component): number;
    destroy(): void;
}
