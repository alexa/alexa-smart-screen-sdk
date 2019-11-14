/*!
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import 'jquery-touchswipe';
import APLRenderer from '../../APLRenderer';
import { Navigation } from '../../enums/Navigation';
import { PropertyKey } from '../../enums/PropertyKey';
import { Component, FactoryFunction, IComponentProperties } from '../Component';
import { ActionableComponent } from '../ActionableComponent';
/**
 * @ignore
 */
export interface IPagerProperties extends IComponentProperties {
    [PropertyKey.kPropertyInitialPage]: number;
    [PropertyKey.kPropertyNavigation]: Navigation;
}
/**
 * @ignore
 */
export declare class PagerComponent extends ActionableComponent<IComponentProperties> {
    /**IComponentProperties
     * The delay (in ms) used between transitions of the pager
     * @type {number}
     */
    static readonly ANIMATION_DELAY: number;
    /**
     * Whether forwards navigation (controlled by the user) is permitted
     */
    private allowForwardsNav;
    /**
     * Whether backwards navigation (controlled by the user) is permitted
     */
    private allowBackwardsNav;
    private pagerContainer;
    private currentPage;
    private navigation;
    private setPagePromise;
    private setPageCallback;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    init(): void;
    setProperties(props: IPagerProperties): void;
    /**
     * Sets the page which is currently displayed by the pager
     * @param {number} value The distance to move
     */
    setPage(value: number): Promise<void>;
    /**
     * Returns the number of items which exist within the pager
     * @returns {number} The number of items
     */
    getPageCount(): number;
    /**
     * Returns the page currently being displayed by this component
     * @returns {number} The currently displayed page
     */
    getCurrentPage(): number;
    stopAnimation(): void;
    /**
     * @override
     * @returns The value of the component, as defined by the APL specification
     * @return {any}
     */
    getValue(): any;
    /**
     * Configures the navigation controls for the pager, only affects navigation controlled
     * directly by the user, navigation through commands is always available
     */
    private configureNavigation();
    focusCurrentPage(event: Event): void;
}
