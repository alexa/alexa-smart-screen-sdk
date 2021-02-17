/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import EventEmitter = require("./../../../eventemitter3/index");
import APLRenderer from '../APLRenderer';
import { PropertyKey } from '../enums/PropertyKey';
import { UpdateType } from '../enums/UpdateType';
import { ILogger } from '../logging/ILogger';
import { GradientSpreadMethod } from '../enums/GradientSpreadMethod';
import { GradientUnits } from '../enums/GradientUnits';
import { IAVGGradient } from './avg/Gradient';
export declare const SVG_NS = "http://www.w3.org/2000/svg";
export declare const uuidv4: any;
export declare const IDENTITY_TRANSFORM = "matrix(1.000000,0.000000,0.000000,1.000000,0.000000,0.000000)";
/**
 * @ignore
 */
export interface IGenericPropType {
    [key: number]: any;
}
/**
 * @ignore
 */
export interface IComponentProperties {
    [PropertyKey.kPropertyOpacity]: number;
    [PropertyKey.kPropertyBounds]: APL.Rect;
    [PropertyKey.kPropertyInnerBounds]: APL.Rect;
    [PropertyKey.kPropertyShadowHorizontalOffset]: number;
    [PropertyKey.kPropertyShadowVerticalOffset]: number;
    [PropertyKey.kPropertyShadowRadius]: number;
    [PropertyKey.kPropertyShadowColor]: number;
}
/**
 * @ignore
 */
export declare type FactoryFunction = (renderer: APLRenderer, component: APL.Component, parent?: Component, ensureLayout?: boolean, insertAt?: number) => Component;
export declare type Executor = () => void;
export declare abstract class Component<PropsType = IGenericPropType> extends EventEmitter {
    renderer: APLRenderer;
    component: APL.Component;
    protected factory: FactoryFunction;
    parent: Component;
    protected logger: ILogger;
    container: HTMLDivElement;
    /**
     * Array of children components in this hierarchy
     */
    children: Component[];
    /** Map of every property */
    props: IGenericPropType;
    /** Absolute calculated bounds of this component */
    bounds: APL.Rect;
    /** Absolute calculated inner bounds of this component */
    innerBounds: APL.Rect;
    /** Component unique ID */
    id: string;
    /** User assigned ID */
    assignedId: string;
    /** true us destroyed was called */
    protected isDestroyed: boolean;
    /** Component state */
    protected state: {
        [UpdateType.kUpdatePagerPosition]: number;
        [UpdateType.kUpdatePressState]: number;
        [UpdateType.kUpdatePressed]: number;
        [UpdateType.kUpdateScrollPosition]: number;
        [UpdateType.kUpdateTakeFocus]: number;
    };
    protected executors: Map<PropertyKey, (props: PropsType) => void>;
    /**
     * @param renderer The renderer instance
     * @param component The core component
     * @param factory Factory function to create new components
     * @param parent The parent component
     * @ignore
     */
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    /**
     * Creates all child components and initialized all calculated properties
     * @ignore
     */
    init(): void;
    protected onPropertiesUpdated(): void;
    /**
     * TODO -> separate layout props from style properties
     * @param props
     * @ignore
     */
    setProperties(props: PropsType): void;
    /**
     * Will update the view with any dirty properties
     * @ignore
     */
    updateDirtyProps(): void;
    /**
     * Call this to set the state of this component and any components
     * that inherit state from it.
     * @param stateProp
     * @param value
     */
    update(stateProp: UpdateType, value: number | string): void;
    /**
     * Destroys and cleans up this instance
     */
    destroy(destroyComponent?: boolean): void;
    /**
     * Converts a number to css rgba format
     * @param val Number value to convert
     */
    static numberToColor(val: number): string;
    static getGradientSpreadMethod(gradientSpreadMethod: GradientSpreadMethod): string;
    static getGradientUnits(gradientUnits: GradientUnits): string;
    static fillAndStrokeConverter(val: object, transform: string, parent: Element, logger: ILogger): string;
    static getGradientUrl(val: IAVGGradient, transform: string, parent: Element, logger: ILogger): string;
    static getPatternUrl(pattern: APL.GraphicPattern, transform: string, parent: Element, logger: ILogger): string;
    hasValidBounds(): boolean;
    static getClipPathElementId(pathData: string, parent: Element): string;
    inflateAndAddChild(index: number, data: string): Component | undefined;
    remove(): boolean;
    protected boundsUpdated(): void;
    protected isLayout(): boolean;
    /**
     * If parent is Container component and this component is layout components then limit size of child to
     * offset+size of parent to overcome broken skills
     */
    protected alignSize(): void;
    protected propExecutor: (executor: () => void, ...props: PropertyKey[]) => any;
    protected getProperties(): PropsType;
    protected setTransform: () => void;
    protected setOpacity: () => void;
    protected setDisplay: () => void;
    protected setBoundsAndDisplay: () => void;
    protected setUserProperties: () => void;
    protected handleComponentChildrenChange: () => void;
    protected getCssShadow: () => string;
    private setShadow;
    protected applyCssShadow: (shadowParams: string) => void;
}
