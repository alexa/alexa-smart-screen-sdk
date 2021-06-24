/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { Component } from './components/Component';
import { MeasureMode } from './components/text/MeasureMode';
import { AnimationQuality } from './enums/AnimationQuality';
import { IVideoFactory } from './components/video/IVideoFactory';
import { AudioPlayerWrapper } from './AudioPlayerWrapper';
import { AudioPlayerFactory } from './media/audio/AudioPlayer';
import { ILogger } from './logging/ILogger';
import { IExtensionManager } from './extensions/IExtensionManager';
/**
 * Device viewport mode
 */
export declare type DeviceMode = 'AUTO' | 'HUB' | 'MOBILE' | 'PC' | 'TV';
/**
 * Device viewport shape
 */
export declare type ViewportShape = 'ROUND' | 'RECTANGLE';
/**
 * Device screen mode
 */
export declare type ScreenMode = 'normal' | 'high-contrast';
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
 * Configuration Change options.
 *
 * Dynamic changes to the renderer viewport or envrionment.
 */
export interface IConfigurationChangeOptions {
    /** Viewport Width in pixels */
    width?: number;
    /** Viewport Height in pixels */
    height?: number;
    /** APL theme. Usually 'light' or 'dark' */
    docTheme?: string;
    /** Device mode. If no provided "HUB" is used. */
    mode?: DeviceMode;
    /** Relative size of fonts to display as specified by the OS accessibility settings */
    fontScale?: number;
    /** The accessibility settings for how colors should be displayed. */
    screenMode?: ScreenMode;
    /** Indicates if a screen reader has been enabled for the user. */
    screenReader?: boolean;
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
 * Event coming from APL to request data fetch for any of registered DataSources.
 */
export interface IDataSourceFetchRequest {
    type: string;
    payload: any;
}
export interface IExtensionEvent {
    uri: string;
    name: string;
    source: any;
    params: any;
    event?: APL.Event;
}
/**
 * keyboard handler type
 */
export declare enum KeyHandlerType {
    KeyDown = 0,
    KeyUp = 1
}
/**
 * Async keyboardEvent.
 */
export interface IAsyncKeyboardEvent extends KeyboardEvent {
    asyncChecked: boolean;
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
    /** Callback for logging PEGTL Parsing Session Error */
    onPEGTLError?: (error: string) => void;
    /** Callback for Finish command */
    onFinish?: () => void;
    /** Callback for Extension command */
    onExtensionEvent?: (event: IExtensionEvent) => Promise<boolean>;
    /** Callback when speakPlayback starts */
    onSpeakEventEnd?: (type: string) => void;
    /** Callback when speakPlayback ends */
    onSpeakEventStart?: (type: string) => void;
    /** Callback for Data Source fetch requests */
    onDataSourceFetchRequest?: (event: IDataSourceFetchRequest) => void;
    /** Callback for pending errors from APLCore Library */
    onRunTimeError?: (pendingErrors: object[]) => void;
    /** Callback for ignoring resize config change */
    onResizingIgnored?: (ignoredWidth: number, ignoredHeight: number) => void;
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
    /** Starting UTC time in milliseconds since 1/1/1970 */
    utcTime: number;
    /** Offset of the local time zone from UTC in milliseconds */
    localTimeAdjustment: number;
}
/**
 * The main renderer. Create a new one with `const renderer = APLRenderer.create(content);`
 */
export default abstract class APLRenderer<Options = {
}> {
    private mOptions;
    private static mappingKeyExpression;
    private static mousePointerId;
    private lastPointerEventTimestamp;
    protected logger: ILogger;
    componentByMappingKey: Map<string, Component>;
    /** A reference to the APL root context */
    context: APL.Context;
    /** Root renderer component */
    top: Component;
    /** A reference to the APL extension manager */
    extensionManager: IExtensionManager;
    /** Configuration Change handler */
    protected handleConfigurationChange: (configurationChangeOption: IConfigurationChangeOptions) => void;
    /** Document set flag for allowing config change driven resizing */
    protected supportsResizing: boolean;
    private configChangeThrottle;
    private isEdge;
    readonly options: Options;
    audioPlayer: AudioPlayerWrapper;
    /**
     * THis constructor is private
     * @param mOptions options passed in through `create`
     * @ignore
     */
    protected constructor(mOptions: IAPLOptions);
    init(metricRecorder?: (m: APL.DisplayMetric) => void): void;
    /**
     * Sets the renderer view size in pixels
     * @param width width in pixels
     * @param height height in pixels
     */
    setViewSize(width: number, height: number): void;
    /**
     * Sets if the renderer supports resizing as defined by the APL document settings
     * @param supportsResizing - True if the document supports resizing.  Defaults to false.
     */
    setSupportsResizing(supportsResizing: boolean): void;
    /**
     * Process Configuration Change. ViewHost will resize/reinflate upon configuration change if supported.
     * @param configurationChangeOptions The configuration change options to provide to core.
     */
    onConfigurationChange(configurationChangeOptions: IConfigurationChangeOptions): void;
    getComponentCount(): number;
    private setBackground(docTheme);
    /**
     * Get developer tool options (if defined)
     */
    getDeveloperToolOptions(): IDeveloperToolOptions | undefined;
    onRunTimeError(pendingErrors: object[]): void;
    onResizingIgnored(ignoredWidth: number, ignoredHeight: number): void;
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
     * Rerender the same template with current content, config and context.
     */
    reRenderComponents(): void;
    /**
     * Cleans up this instance
     */
    destroy(preserveContext?: boolean): void;
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
     * @returns true if is in screenLock state, false otherwise.
     */
    screenLock(): boolean;
    /**
     * Cancel Animation Frame
     */
    stopUpdate(): Promise<void>;
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
    /**
     * Destroy current rendering component from top.
     */
    destroyRenderingComponents(): void;
    private getScreenCoords;
    private getLeavingCoords;
    private getTransformScale;
    private getViewportCoords;
    private onPointerDown;
    private onPointerMove;
    private onPointerUp;
    private onPointerLeave;
    private sendMousePointerEvent;
    private sendTouchPointerEvent;
    private handleKeyDown;
    private handleKeyUp;
    /**
     * Get APL.Keyboard object
     * for MS edge Gamepad, repeat/altKey/ctrlKey/metaKey/shiftKey are undefined, need set default to false
     */
    private getKeyboard;
    private getKeyboardCodeInEdge;
    private passKeyboardEventToCore;
    private isDPadKey;
    private renderComponents();
    private removeRenderingComponents();
    private focusTopLeft();
    private passWindowEventsToCore;
}
