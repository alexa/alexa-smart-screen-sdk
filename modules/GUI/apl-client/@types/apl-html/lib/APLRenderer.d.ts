import { Component } from './components/Component';
import { MeasureMode } from './components/text/MeasureMode';
import { AnimationQuality } from './enums/AnimationQuality';
import { IVideoFactory } from './components/video/IVideoFactory';
import { AudioPlayerWrapper } from './AudioPlayerWrapper';
import { AudioPlayerFactory } from './media/audio/AudioPlayer';
import { ILogger } from './logging/ILogger';
/**
 * Device viewport mode
 */
export declare type DeviceMode = 'AUTO' | 'HUB' | 'MOBILE' | 'PC' | 'TV';
/**
 * Device viewport shape
 */
export declare type ViewportShape = 'ROUND' | 'RECTANGLE';
/**
 * Physical charcteristics of the viewport
 */
export interface IViewportCharacteristics {
    /** Width in pixels */
    width: number;
    /** Height in pixels */
    height: number;
    /** `true` if the screen is round */
    isRound: boolean;
    /** Viewport shape. If undefined than decided by "isRound" */
    shape?: ViewportShape;
    /** Dots per inch */
    dpi: number;
}
/**
 * Environment and support options
 */
export interface IEnvironment {
    /** Agent Name */
    agentName: string;
    /** Agent Version */
    agentVersion: string;
    /** `true` if OpenURL command is supported. Defaults to `false` */
    allowOpenUrl?: boolean;
    /** `true` if video is not supported. Defaults to `false` */
    disallowVideo?: boolean;
    /** Level of animation quality. Defaults to `AnimationQuality.kAnimationQualityNormal` */
    animationQuality?: AnimationQuality;
}
/**
 * Developer tool options can be used to inject additional data into the DOM
 *
 * Keys are defined post transformation, for e.g. if the input is '-user-foo', then
 * define keys as 'foo' here
 */
export interface IDeveloperToolOptions {
    /** Key to use to create component mapping */
    mappingKey: string;
    /** Keys to export as data- attributes in the DOM */
    writeKeys: string[];
}
/**
 * Event coming from APL.
 * See https://aplspec.aka.corp.amazon.com/release-1.1/html/standard_commands.html#user-event for more information.
 */
export interface ISendEvent {
    source: any;
    arguments: string[];
    components: any[];
}
/**
 * keyboard handler type
 */
export declare enum KeyHandlerType {
    KeyDown = 0,
    KeyUp = 1
}
/**
 * Options when creating a new APLRenderer
 */
export interface IAPLOptions {
    /** Contains all the information on environment suport and options */
    environment: IEnvironment;
    /** APL theme. Usually 'light' or 'dark' */
    theme: string;
    /** Optional Video player factory. If no player is provided, a default player will be used */
    videoFactory?: IVideoFactory;
    /** The HTMLElement to draw onto */
    view: HTMLElement;
    /** Physical viewport characteristics */
    viewport: IViewportCharacteristics;
    /** Device mode. If no provided "HUB" is used. */
    mode?: DeviceMode;
    /** Optional externalized audio player */
    audioPlayerFactory?: AudioPlayerFactory;
    /** Callback for executed SendEvent commands */
    onSendEvent?: (event: ISendEvent) => void;
    /**
     * Callback when a AVG source needs to be retreived by the consumer
     * If this is not provided, this viewhost will use the fetch API to
     * retreive graphic content from sources.
     */
    onRequestGraphic?: (source: string) => Promise<string | undefined>;
    /**
     * Callback to open a URL. Return `false` if this call fails
     */
    onOpenUrl?: (source: string) => Promise<boolean>;
    /**
     * Contains developer tool options
     */
    developerToolOptions?: IDeveloperToolOptions;
}
/**
 * The main renderer. Create a new one with `const renderer = APLRenderer.create(content);`
 */
export default abstract class APLRenderer<Options = {
}> {
    private mOptions;
    private static mappingKeyExpression;
    protected logger: ILogger;
    componentByMappingKey: Map<string, Component>;
    /** A reference to the APL root context */
    context: APL.Context;
    /** Root renderer component */
    top: Component;
    readonly options: Options;
    audioPlayer: AudioPlayerWrapper;
    /**
     * THis constructor is private
     * @param mOptions options passed in through `create`
     * @ignore
     */
    protected constructor(mOptions: IAPLOptions);
    init(): void;
    /**
     * Get developer tool options (if defined)
     */
    getDeveloperToolOptions(): IDeveloperToolOptions | undefined;
    /**
     * Called by core when a text measure is required
     * @param component The component to measure
     * @param measureWidth specified width
     * @param widthMode Mode to measure width
     * @param measureHeight specified height
     * @param heightMode Mode to measure height
     * @ignore
     */
    onMeasure(component: APL.Component, measureWidth: number, widthMode: MeasureMode, measureHeight: number, heightMode: MeasureMode): {
        width: number;
        height: number;
    };
    /**
     * Baseline
     * @param component The component to measure
     * @param width specified width
     * @param height specified height
     * @ignore
     */
    onBaseline(component: APL.Component, width: number, height: number): number;
    /**
     * Cleans up this instance
     */
    destroy(): void;
    /**
     * @ignore
     */
    getBackgroundColor(): string;
    /**
     * Gets a component by its developer assigned ID.
     * @param id The developer assigned component ID
     */
    getComponentById(id: string): Component;
    /**
     * Return a map of components where the key matches the non-unique part of mappingKey
     * (when mappings are created a unique identifier is appended to ensure maps are unique)
     *
     * Note: mappingKeys are configured via developerToolOptions and come from -user- attributes
     */
    getComponentsByMappingKey(mappingKey: string): Map<string, Component>;
    /**
     * Add a component without re-rendering the whole output. The virtual component will be returned.
     *
     * @param parent Virtual component to add new component to.
     * @param childIndex Index to put component to. Existing component at this index will be pushed up.
     * @param componentData json string containing component definition.
     * @returns virtual component.
     */
    addComponent(parent: Component, childIndex: number, componentData: string): Component | undefined;
    /**
     * Delete a component without re-rendering the whole output.
     *
     * @param component Virtual component to remove.
     * @returns true if removed, false otherwise.
     */
    deleteComponent(component: Component): boolean;
    /**
     * Update a component without re-rendering the whole output. Given the component's path and json payload,
     * this component's DOM element will be returned.
     *
     * @param parent Virtual component to replace current component with.
     * @param componentData Json string containing component definition.
     * @returns virtual component.
     */
    updateComponent(component: Component, componentData: string): Component | undefined;
    private getScreenCoords;
    private getTransformScale;
    private getViewportCoords;
    private onMouseMove;
    private onMouseLeave;
    private handleKeyDown;
    private handleKeyUp;
    private getKeyboard;
}
