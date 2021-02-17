export declare enum EventType {
    /**
     * Control media
     *
     * kEventPropertyCommand: The command to execute
     * kEventPropertyValue: The integer value associated with that command
     *
     * Does not have an ActionRef.
     */
    kEventTypeControlMedia = 0,
    /**
     * Change the focus
     *
     * Does not have an ActionRef.
     */
    kEventTypeFocus = 1,
    /**
     * Request a URL to be opened
     *
     * kEventPropertySource: The URL to open.
     *
     * The server must resolve the ActionRef if the URL is opened.
     * The server should resolve the ActionRef with a non-zero argument
     * if the URL fails to open.
     */
    kEventTypeOpenURL = 2,
    /**
     * Play media
     *
     * kEventPropertyAudioTrack: The audio track we should play this media on.
     * kEventPropertySource: An array of media sources
     *
     * The server must resolve the ActionRef when the audio track is set to foreground.
     * If the audio track is background or none, the ActionRef is not provided.
     */
    kEventTypePlayMedia = 3,
    /**
     * Warn the view host that a speak event is coming.
     *
     * kEventPropertySource: The speech URI.
     *
     * Does not have an ActionRef.
     */
    kEventTypePreroll = 4,
    /**
     * Requests the bounds information for a text component
     *
     * The component is a TextComponent that needs the first line bounds measured
     */
    kEventTypeRequestFirstLineBounds = 5,
    /**
     * Scroll a component into view.
     *
     * The component is the component to scroll.
     * kEventPropertyPosition: The scroll position or page to change to.
     *
     * The server must resolve the ActionRef when the scroll is completed.
     */
    kEventTypeScrollTo = 6,
    /**
     * Send an event to the server
     *
     * kEventPropertySource: The rich source object describing who raised this event.
     * kEventPropertyArguments: The argument array provided by the APL author
     * kEventPropertyComponents: The values of the components requested by the APL author
     *
     * Does not have an ActionRef
     */
    kEventTypeSendEvent = 7,
    /**
     * Change the page in a pager.
     *
     * The component is the pager.
     * kEventPropertyPosition: The page to switch to (integer)
     * kEventPropertyDirection: The direction to move. Either kEventDirectionForward or kEventDirectionBackward
     *
     * The server must resolve the ActionRef when the scroll is completed.
     */
    kEventTypeSetPage = 8,
    /**
     * Speak a single component.
     *
     * kEventPropertyHighlightMode: Highlight mode. kEventHighlightModeLine or kEventHighlightModeBlock
     * kEventPropertySource: The speech URI.
     *
     * The server must resolve the ActionRef when the scroll is completed.
     */
    kEventTypeSpeak = 9,
    /**
     * Send a finish command.
     *
     * kEventPropertyReason: The reason for the finish command. kEventReasonExit or kEventReasonBack
     *
     * Does not have an ActionRef
     */
    kEventTypeFinish = 10,
    /**
     * A extension event registered with the core engine by the view host.
     */
    kEventTypeExtension = 11,
    /**
     * DataSourceProvider created event that could be used for data fetch requests.
     *
     * kEventPropertyName: name (type) of datasource that requests a fetch.
     * kEventPropertyValue: implementation specific fetch request.
     *
     * Does not have an ActionRef
     */
    kEventTypeDataSourceFetchRequest = 12
}
