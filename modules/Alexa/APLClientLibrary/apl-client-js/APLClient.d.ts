/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
export interface RenderingOptionsPayload {
    legacyKaraoke: boolean;
}
export interface IComponentPayload {
    children: IComponentPayload[];
    id: string;
    type: number;
    [key: string]: any;
}
export interface MeasurePayload extends IComponentPayload {
    width: number;
    widthMode: number;
    height: number;
    heightMode: number;
}
export interface EventPayload {
    type: number;
    id: string;
    [key: string]: any;
}
export interface EventTerminatePayload {
    token: number;
}
export interface ScalingPayload {
    scaleFactor: number;
    viewportWidth: number;
    viewportHeight: number;
}
export interface BaselinePayload extends IComponentPayload {
    width: number;
    height: number;
}
export interface DocThemePayload {
    docTheme: string;
}
export interface BackgroundPayload {
    background: APL.IBackground;
}
export interface ScreenLockPayload {
    screenLock: boolean;
}
export interface IsCharacterValidPayload {
    componentId: string;
    messageId: string;
    valid: boolean;
}
export interface OnHandleKeyboardPayload {
    messageId: string;
    result: boolean;
}
export interface LocaleMethodPayload {
    method: string;
    locale: string;
    value: string;
}
export interface FocusableAreasPayload {
    messageId: string;
    areas: Map<string, APL.Rect>;
}
export interface DisplayedChildCountPayload {
    componentId: string;
    messageId: string;
    displayedChildCount: number;
}
export interface DisplayedChildIdPayload {
    componentId: string;
    messageId: string;
    displayedChildId: string;
}
export interface FocusedPayload {
    messageId: string;
    result: string;
}
export interface SupportsResizingPayload {
    supportsResizing: boolean;
}
export interface PayloadTypeMap {
    "renderingOptions": RenderingOptionsPayload;
    "measure": MeasurePayload;
    "hierarchy": IComponentPayload;
    "reHierarchy": IComponentPayload;
    "scaling": ScalingPayload;
    "event": EventPayload;
    "dirty": IComponentPayload[];
    "eventTerminate": EventTerminatePayload;
    "baseline": BaselinePayload;
    "docTheme": DocThemePayload;
    "background": BackgroundPayload;
    "screenLock": ScreenLockPayload;
    "ensureLayout": string;
    "isCharacterValid": IsCharacterValidPayload;
    "handleKeyboard": OnHandleKeyboardPayload;
    "localeMethod": LocaleMethodPayload;
    "getFocusableAreas": FocusableAreasPayload;
    "getFocused": FocusedPayload;
    "getDisplayedChildCount": DisplayedChildCountPayload;
    "getDisplayedChildId": DisplayedChildIdPayload;
    "supportsResizing": SupportsResizingPayload;
}
export interface Message<Type extends keyof PayloadTypeMap> {
    type: Type;
    seqno: number;
    payload: PayloadTypeMap[Type];
}
export interface APLCLientEventTypeMap {
    "close": CloseEvent;
    "error": Event;
    "open": Event;
}
export interface IAPLClientListener {
    onOpen?(): void;
    onClose?(): void;
    onError?(): void;
}
export interface IAPLMessageListener {
    onMeasure?(message: Message<"measure">): void;
    onRenderingOptions?(message: Message<"renderingOptions">): void;
    onHierarchy?(message: Message<"hierarchy">): void;
    onReHierarchy?(message: Message<"reHierarchy">): void;
    onScaling?(message: Message<"scaling">): void;
    onDirty?(message: Message<"dirty">): void;
    onEvent?(message: Message<"event">): void;
    onEventTerminate?(message: Message<"eventTerminate">): void;
    onBaseline?(message: Message<"baseline">): void;
    onDocTheme?(message: Message<"docTheme">): void;
    onBackground?(message: Message<"background">): void;
    onScreenLock?(message: Message<"screenLock">): void;
    onEnsureLayout?(message: Message<"ensureLayout">): void;
    onIsCharacterValid?(message: Message<"isCharacterValid">): void;
    onHandleKeyboard?(message: Message<"handleKeyboard">): void;
    onLocaleMethod?(message: Message<"localeMethod">): void;
    onGetFocusableAreas?(message: Message<"getFocusableAreas">): void;
    onGetFocused?(message: Message<"getFocused">): void;
    onGetDisplayedChildCount?(message: Message<"getDisplayedChildCount">): void;
    onGetDisplayedChildId?(message: Message<"getDisplayedChildId">): void;
    onSupportsResizing?(message: Message<"supportsResizing">): void;
}
/**
 * Extend this class to implement a client. Must implement events described in
 * `IAPLClient`
 */
export declare abstract class APLClient {
    constructor();
    /**
     * Override this method to send a message
     * @param message
     */
    abstract sendMessage(message: any): any;
    /**
     * Adds a lifecycles listener
     * @param listener
     */
    addListener(listener: IAPLClientListener): void;
    /**
     * Removes a lifecycles listener
     * @param listener
     */
    removeListener(listener: IAPLClientListener): void;
    /**
     * Removes all lifecycles listeners
     */
    removeAllListeners(): void;
    /**
     * Call this when the client return isCharacterValid message with result;
     * @param message
     */
    protected isCharacterValid(message: Message<"isCharacterValid">): void;
    /**
     * Call this when the client return getDisplayedChildCount message with result
     * @param message
     */
    protected getDisplayedChildCount(message: Message<"getDisplayedChildCount">): void;
    /**
     * Call this when the client return getDisplayedChildId message with result
     * @param message
     */
    protected getDisplayedChildId(message: Message<"getDisplayedChildId">): void;
    /**
     * Call this when the client return handleKeyboard message with result;
     * @param message
     */
    protected handleKeyboard(message: Message<"handleKeyboard">): void;
    /**
     * Call this when the client return supportsResizing message with result;
     * @param message
     */
    protected supportsResizing(message: Message<"supportsResizing">): void;
    /**
     * Call this when the client receives a message from the server
     */
    onMessage<P extends keyof PayloadTypeMap>(message: Message<P>): void;
    /**
     * Call this from  subclass when a client connection is closed
     */
    protected onClose(): void;
    /**
     * Call this from a subclass when a client connection is opened
     */
    protected onOpen(): void;
    /**
     * Call this from a subclass when there is a connection error
     */
    protected onError(): void;
}
