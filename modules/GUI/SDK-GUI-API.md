# Alexa Smart Screen SDK GUI API Specification

Server refers to the SDK, and client refers to the process initiating the connection. Messages are JSON objects encoded as strings.

All messages (in any direction) have a basic structure, the *type* attribute indicates what the contents of the message are.

```javascript
{
  type: string,
  ...
}
```

# Server -> Client (Inbound)

## initRequest

This message is sending initialization data to the GUI client and expecting an `initResponse` message back.

```javascript
{
    type: 'initRequest',
    smartScreenSDKVersion: string
}
```

## guiConfiguration

This message is sending configuration data to the GUI client to initialize window presentation and functionality, and expecting a `deviceWindowState` message back.

```javascript
{
    type: 'guiConfiguration',
    payload: {
      visualCharacteristics: {},
      appConfig: {}
    }
}
```

## alexaStateChanged

This message relays alexa state transitions occurring within the SDK which the GUI Client should handle for presentation of UI element like voice chrome.

```javascript
enum AlexaState {
  UNKNOWN = 'UNKNOWN',
  DISCONNECTED = 'DISCONNECTED',
  CONNECTING = 'CONNECTING',
  CONNECTED = 'CONNECTED',
  IDLE = 'IDLE',
  LISTENING = 'LISTENING',
  EXPECTING = 'EXPECTING',
  THINKING = 'THINKING',
  SPEAKING = 'SPEAKING'
}

{
  type: 'alexaStateChanged',
  state: AlexaState
}
```

## onFocusChanged

This message provides the GUI Client with Focus state changes on corresponding channel.

```javascript
{
    type: 'onFocusChanged',
    token: number,
    channelState: string
}
``` 

## focusResponse

This message provides the GUI Client with the result of `focusAcquireRequest` and `focusReleaseRequest` requests processing.

```javascript
{
    type: 'focusResponse',
    token: number,
    result: boolean
}
``` 

## requestAuthorization

Tis message provides the GUI Client with information to present to the user to complete CBL device authorization.

See : https://developer.amazon.com/docs/alexa-voice-service/setup-authentication.html#code-based-screens

```javascript
{
    type: 'requestAuthorization',
    url : string,
    code : string,
    clientId : string
}
```

## authorizationChange

Tis message provides the GUI Client with information about changes to the state of authorization.

```javascript
type AuthorizationState =
    'UNINITIALIZED'
    | 'REFRESHED'
    | 'EXPIRED'
    | 'ERROR';
    
{
    type: 'authorizationChange',
    state : AuthorizationState
}
```

## renderTemplate

This message instructs the GUI Client to draw visual metadata to the screen. *Payload* is an arbitrary object that contains the datasource to render.

See : https://developer.amazon.com/docs/alexa-voice-service/templateruntime.html#rendertemplate

```javascript
{
  type: 'renderTemplate',
  payload: {}
}
```

## clearTemplateCard

This message instructs the client to clear visual content from the screen.

```javascript
{
  type: 'clearTemplateCard'
}
```

## renderPlayerInfo

This message instructs the GUI Client to display visual metadata associated with a media item, such as a song or playlist. The GUI Client should bind the metadata to a template and render for the end user.

See : https://developer.amazon.com/docs/alexa-voice-service/templateruntime.html#renderplayerinfo

```javascript
{
  type: 'renderPlayerInfo',
  payload: {}
}
```

## clearPlayerInfoCard

This message instructs the client to clear visual content from the screen.

```javascript
{
  type: 'clearPlayerInfoCard'
}
```

## aplRender

This message instructs the GUI Client to start an APL render in the targeted window.
The APL renderer will use the `aplEvent` message to poll the SDK for drawing data delivered via `aplCore` messages.

```javascript
{
  type: 'aplRender',
  windowId : string,
  token : string
}
```

## aplCore

This message instructs the GUI Client to send drawing updates to the active APL renderer. *Payload* is an arbitrary object containing drawing data for the APL renderer.

```javascript
{
  type: 'aplCore'
  payload: {}
}
```

## clearDocument

This message instructs the client to clear visual content from the screen.

```javascript
{
  type: 'clearDocument'
}
```

# Client -> Server (Outbound)

## initResponse

This message is sent as a response to an `initRequest` message and contains whether the given SDK version is supported and the maximum APL version supported by the client.

```javascript
{
    type: 'initResponse',
    isSupported: boolean,
    APLMaxVersion: string
}
```

## deviceWindowState

This message is sent as a response to a `guiConfiguration` message, or reported to the SDK on any client side change to available window states.  It contains data defining the state of the targetable windows in the client at time of report.

```javascript
{
    type: 'deviceWindowState',
    payload: {}
}
```

## focusAcquireRequest

This message is sent from FocusManager in order to request acquisition of a channel.

```javascript
{
    type: 'focusAcquireRequest',
    token: number,
    channelName: string
}
``` 

## focusReleaseRequest

This message is sent from FocusManager in order to request release of the previously acquired channel.

```javascript
{
    type: 'focusReleaseRequest',
    token: number,
    channelName: string
}
``` 

## onFocusChangedReceivedConfirmation

This message is sent from GUI Client to confirm that `onFocusChanged` message was received.

```javascript
{
    type: 'onFocusChangedReceivedConfirmation',
    token: number
}
```

## tapToTalk

This message instructs the SDK to open up the microphone and listen for an utterance using the `TAP` initiator.

See : https://developer.amazon.com/docs/alexa-voice-service/speechrecognizer.html#initiator

```javascript
{
  type: 'tapToTalk'
}
```

## holdToTalk

This message instructs the SDK to open up the microphone and listen for an utterance using the `PRESS_AND_HOLD` initiator.

See : https://developer.amazon.com/docs/alexa-voice-service/speechrecognizer.html#initiator

```javascript
{
  type: 'holdToTalk'
}
```

## executeCommands

This message instructs the SDK to execute APL commands, sent from the GUI Client, in the active APL Renderer. *Payload* is an arbitrary object that contains one or more commands.  The SDK will respond with `aplCore` messages.

See : https://developer.amazon.com/docs/alexa-presentation-language/apl-execute-command-directive.html

```javascript
{
  type: 'executeCommands',
  token: string,
  payload: {}
}
```

## renderStaticDocument

This message instructs the SDK to render the contained APL document and datasource, sent from the GUI Client, in the specified APL Renderer window. *Payload* is an arbitrary object that contains the document, datasources, and supportedViewports.  The SDK will respond with `aplCore` messages.

See : https://developer.amazon.com/docs/alexa-presentation-language/apl-render-document-skill-directive.html

```javascript
{
  type: 'renderStaticDocument',
  token: string,
  windowId: string,
  payload: {}
}
```

## aplEvent

This message handles all communication with the APL Core Engine, sending interaction and UI update events from the active APL Renderer to the SDK.  *Payload* is an arbitrary object containing the event data.  The SDK will respond with `aplCore` messages.

```javascript
{
    type: 'aplEvent',
    payload: {}
}
```

## activityEvent

This message is sent to the SDK to provide the current activity state of the visual response in the GUI Client.

```javascript
type ActivityEvent =
    'ACTIVATED'
    | 'DEACTIVATED'
    | 'ONE_TIME'
    | 'INTERRUPT'
    | 'UNKNOWN';

{
    type: 'activityEvent',
    event: ActivityEvent
}
```

## navigationEvent

This message is sent to the SDK to indicate a naviagational event triggered in the GUI Client through use of a remote or other physical button input.

```javascript
type NavigationEvent =
    'EXIT'
    | 'BACK'
    | 'UNKNOWN';

{
    type: 'navigationEvent',
    event: NavigationEvent
}
```

## logEvent

This message is sent to the SDK to provide logging from the GUI Client.

```javascript
type LogLevel =
    'trace'
    | 'debug'
    | 'info'
    | 'warn'
    | 'error';

{
    type: 'logEvent',
    level : LogLevel,
    component : string,
    message : string
}
```
