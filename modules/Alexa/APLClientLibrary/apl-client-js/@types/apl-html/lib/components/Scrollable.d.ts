/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { ScrollDirection } from '../enums/ScrollDirection';
import { IComponentProperties } from './Component';
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
    protected listenerAdded: boolean;
    protected keyUp: boolean;
    protected spatialNavigationSection: string;
    protected hasFocusableChildren: boolean;
    protected startGap: HTMLDivElement;
    protected $startGap: JQuery<HTMLDivElement>;
    protected endGap: HTMLDivElement;
    protected $endGap: JQuery<HTMLDivElement>;
    private skipNextUpdate;
    private scrollHandler;
    private gapCount;
    protected isLayout(): boolean;
    scrollToPosition(position: number, velocity: number): Promise<void>;
    /**
     * Scrolls by the number of pages
     * @param pages Number of pages to scroll
     * @param velocity Velocity in pages per second
     */
    scroll(pages: number, velocity: number): Promise<void>;
    /**
     * Called by a command to stop scrolling
     */
    stopScroll(): void;
    protected registerScrollHandler(handler: ScrollHandler): void;
    protected allowFocus(requestedDistance: number, moveTo: HTMLDivElement): boolean;
    /**
     * Callback to figure out if focusing next target component allowed to SpatialNavigation.
     * Calculates distance to be traveled to get child into focus and decides if any other action should be taken.
     *
     * @param parent Top component controlling focusables.
     * @param id Scroll-to component ID.
     */
    protected moveAllowed(parent: Scrollable, id: string): boolean;
    protected initScrollGap(gap: HTMLElement, sectionId: string): void;
    protected overrideSpatialNavigationSection(container: Element): boolean;
    configureNavigation(): void;
    private scrollComponentInView(id);
    getScrollPosition(): number;
    getScrollLength(): number;
    getPageSize(): number;
    getDirection(): ScrollDirection;
    private adjustVisible(offset, pageSize, componentSize);
}
