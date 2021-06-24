/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
export declare enum EventType {
    /**     * Control media     *     * kEventPropertyCommand: The command to execute     * kEventPropertyValue: The integer value associated with that command     *     * Does not have an ActionRef.     */
    kEventTypeControlMedia = 0,
    /**     * Change the focus     *     * Does not have an ActionRef.     *     * With kExperimentalFeatureHandleFocusInCore enabled:     * Notifies server about acquired or lost focus. Have Component reference if focus acquired, don't if releasing.     *     * kEventPropertyValue: Rect representing bounds of focused component.     * kEventPropertyDirection: Focus movement direction (in case of focus releasing).     *     * Doesn't have an ActionRef in case of acquired focus, have one in case of releasing that should be resolved with true     * if focus should be released and false if docus should stay where it is.     */
    kEventTypeFocus = 1,
    /**     * Request a URL to be opened     *     * kEventPropertySource: The URL to open.     *     * The server must resolve the ActionRef if the URL is opened.     * The server should resolve the ActionRef with a non-zero argument     * if the URL fails to open.     */
    kEventTypeOpenURL = 2,
    /**     * Play media     *     * kEventPropertyAudioTrack: The audio track we should play this media on.     * kEventPropertySource: An array of media sources     *     * The server must resolve the ActionRef when the audio track is set to foreground.     * If the audio track is background or none, the ActionRef is not provided.     */
    kEventTypePlayMedia = 3,
    /**     * Warn the view host that a speak event is coming.     *     * kEventPropertySource: The speech URI.     *     * Does not have an ActionRef.     */
    kEventTypePreroll = 4,
    /**     * Requests the bounds information for a text component     *     * The component is a TextComponent that needs the first line bounds measured     */
    kEventTypeRequestFirstLineBounds = 5,
    /**     * Scroll a component into view.     *     * The component is the component to scroll.     * kEventPropertyPosition: The scroll position or page to change to.     *     * The server must resolve the ActionRef when the scroll is completed.     */
    kEventTypeScrollTo = 6,
    /**     * Send an event to the server     *     * kEventPropertySource: The rich source object describing who raised this event.     * kEventPropertyArguments: The argument array provided by the APL author     * kEventPropertyComponents: The values of the components requested by the APL author     *     * Does not have an ActionRef     */
    kEventTypeSendEvent = 7,
    /**     * Change the page in a pager.     *     * The component is the pager.     * kEventPropertyPosition: The page to switch to (integer)     * kEventPropertyDirection: The direction to move. Either kEventDirectionForward or kEventDirectionBackward     *     * The server must resolve the ActionRef when the scroll is completed.     */
    kEventTypeSetPage = 8,
    /**     * Speak a single component.     *     * kEventPropertyHighlightMode: Highlight mode. kEventHighlightModeLine or kEventHighlightModeBlock     * kEventPropertySource: The speech URI.     *     * The server must resolve the ActionRef when the scroll is completed.     */
    kEventTypeSpeak = 9,
    /**     * Send a finish command.     *     * kEventPropertyReason: The reason for the finish command. kEventReasonExit or kEventReasonBack     *     * Does not have an ActionRef     */
    kEventTypeFinish = 10,
    /**     * A extension event registered with the core engine by the view host.     */
    kEventTypeExtension = 11,
    /**     * DataSourceProvider created event that could be used for data fetch requests.     *     * kEventPropertyName: name (type) of datasource that requests a fetch.     * kEventPropertyValue: implementation specific fetch request.     *     * Does not have an ActionRef     */
    kEventTypeDataSourceFetchRequest = 12,
    /**     * The Document is asking to be reinflated.  The server (view host) should do one of the following:     *     * 1.  Leave the ActionRef unresolved and call RootContext::reinflate() to reinflate the document.     *     The ActionRef will be terminated and can be ignored.     * 2.  Resolve the ActionRef.  The RootContext will resize() the document if the screen size has changed     *     and continue normal command processing.     *     * No properties     *     * Has an ActionRef.     *     * Note: It is not necessary to resolve the ActionRef if the server is calling RootContext::reinflate()     * because all currently running command sequences will be terminated including the current ActionRef.     */
    kEventTypeReinflate = 13,
    /**     * The Document is asking for external media to be loaded. Only issued when     * @c ExperimentalFeature::kExperimentalFeatureManageMediaRequests is enabled.     *     * kEventPropertySource: the source URI of the requested media     *     * Does not have an ActionRef     *     * Note: Runtime supposed to answer with a call to RootContext::mediaLoaded when media loaded.     */
    kEventTypeMediaRequest = 14
}
