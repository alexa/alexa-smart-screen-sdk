/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../../APLRenderer';
import { PropertyKey } from '../../enums/PropertyKey';
import { Component, FactoryFunction, IComponentProperties } from '../Component';
import { ActionableComponent } from '../ActionableComponent';
/**
 * @ignore
 */
export interface IPagerProperties extends IComponentProperties {
    [PropertyKey.kPropertyInitialPage]: number;
    [PropertyKey.kPropertyCurrentPage]: number;
}
/**
 * @ignore
 */
export declare class PagerComponent extends ActionableComponent<IPagerProperties> {
    static readonly DIRECTIONAL_CACHE_PAGES: number;
    private currentPage;
    private navigation;
    private childCache;
    private wrap;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    /**
     * This will create the item from core and insert at a particular position in the child of pager div
     * @param index Index of the element to create
     * @param insertAt Position of the new div
     * @param initialPage Is it called by the init()? - If so make the div visible
     * @param updatePage Is it called by updatePage()? - If so createItem regradless if it's in the list
     */
    private createItem(index, insertAt, initialPage?, updatePage?);
    /**
     * ensurePage will create and layout the pager component (and it's children) for pages
     * one above and one below the current page.
     */
    private setCurrentPage();
    private updateVisibility();
    private isDisplayed(id);
    private setPages(pagesToCache);
    private updateCache(pagesToCache);
    init(): Promise<void>;
    /**
     * This will be called whenever the page index has changed or any properies so we need to ensure pages again
     * @param props New properties from the CORE
     */
    setProperties(props: IPagerProperties): Promise<void>;
    /**
     * Returns the page currently being displayed by this component
     * @returns {number} The currently displayed page
     */
    getCoreCurrentPage(): number;
    /**
     * @override
     * @returns The value of the component, as defined by the APL specification
     * @return {any}
     */
    getValue(): any;
}
