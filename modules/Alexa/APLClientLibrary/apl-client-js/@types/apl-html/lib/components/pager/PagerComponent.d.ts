/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
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
    [PropertyKey.kPropertyCurrentPage]: number;
}
/**
 * @ignore
 */
export declare class PagerComponent extends ActionableComponent<IPagerProperties> {
    /**
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
    private currentPage;
    private navigation;
    private bufferSize;
    private childCache;
    private memArraySize;
    private setPagePromise;
    private setPageCallback;
    private lastExitAttemptDirection;
    private wrap;
    private xAxis;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    protected isLayout(): boolean;
    /**
     * To figure out where the user's starting point is to find out if to slide LEFT or RIGHT
     * @param e User's binding event
     */
    private initialPoint(e);
    /**
     * This will animate the page slide from previous to next
     * @param pageFrom The div's index of which page it's transition from
     * @param pageTo The div's index of which page it's transition to
     * @param shiftToLeft To proceed to the next page
     */
    private animate(pageFrom, pageTo, shiftToLeft);
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
    private ensurePages();
    /**
     * Add to childCache, however before adding we need to remove exiisting
     * pages that might be store on cache
     * @param index Index of the item to be replaced
     * @param item Item to be replaced
     */
    private updateCacheDiv(index, item);
    /**
     * Clean the child cache when the current page is removed.
     */
    private cleanCache();
    /**
     * Remove page/div/section from
     *  - childCache
     *  - div container
     *  - SpatialNavigation
     * @param index index to be removed
     * @param nextDiv this is required for ensurePage since we will be creating
     * a new item initially, so we need to remove the next item in place.
     */
    private removePage(index, nextDiv?);
    /**
     * Once user has transitioned to another page we will need to update the buffer list
     * This will ensureLayout on the page above and below + wrap
     * @param index The new page that needs to be injected
     * @param shiftToLeft To proceed to the next page
     */
    injectPages(index: number, shiftToLeft: boolean): void;
    /**
     * User's touch swiping binding
     * @param e User's binding event
     */
    touchSwipe(e: any): void;
    /**
     * Once user has swiped this will be called and will animate accordingly along with updating the page
     * @param shiftToLeft To proceed to the next page
     */
    private swipePage(shiftToLeft);
    init(): Promise<void>;
    /**
     * This will be called whenever the page index has changed or any properies so we need to ensure pages again
     * @param props New properties from the CORE
     */
    setProperties(props: IPagerProperties): Promise<void>;
    /**
     * Return an array of all the focusable element within the component
     * @param parent parent element to start from
     */
    private getAffectedSections(parent);
    /**
     * Sets the page which is currently displayed by the pager
     * @param {number} value The distance to move
     */
    setPage(value: number): Promise<void>;
    /**
     * Returns the page currently being displayed by this component
     * @returns {number} The currently displayed page
     */
    getCurrentPage(): number;
    /**
     * @override
     * @returns The value of the component, as defined by the APL specification
     * @return {any}
     */
    getValue(): any;
    focusCurrentPage(event: Event): void;
}
