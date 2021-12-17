/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

import * as React from 'react';
import './aplRendererWindow.css';
import {
  APLClient,
  APLWSRenderer,
  IAPLWSOptions,
  AudioPlayer,
  IAudioEventListener,
  IEnvironment,
  DisplayState
} from 'apl-client';
import {IClient} from '../lib/messages/client';
import {IAPLCoreMessage, IAPLEventMessage} from '../lib/messages/messages';
import {IFocusManager} from '../lib/focus/IFocusManager';
import {AVSAudioPlayer} from '../lib/media/AVSAudioPlayer';
import {AVSVideoFactory} from '../lib/media/AVSVideo';
import {IActivityTracker} from '../lib/activity/IActivityTracker';
import {Timer} from '../lib/utils/timer';
import {IAPLRendererWindowConfig} from '../lib/config/IDeviceAppConfig';
import {ChannelName} from '../lib/focus/ChannelName';
import {IChannelObserver} from '../lib/focus/IChannelObserver';
import {FocusState} from '../lib/focus/FocusState';
import {AVSInterface} from '../lib/focus/AVSInterface';
import {ContentType} from '../lib/focus/ContentType';

type VisibilityProperty = import('csstype').Property.Visibility;

const OVERLAY_WINDOW_ACTIVE_IN_MS : number = 750;
const OVERLAY_WINDOW_INACTIVE_IN_MS : number = 250;
const WINDOW_TRANSITION_IN_MS : number = 250;

export enum APLRendererWindowState {
  INACTIVE = 0,
  ACTIVE = 1
}

export interface IAPLRendererWindowProps {
  id : string;
  windowConfig : IAPLRendererWindowConfig;
  clearRenderer : boolean;
  displayState? : DisplayState;
  client : WebsocketConnectionWrapper;
  refreshRenderer : boolean;
  focusManager? : IFocusManager;
  activityTracker? : IActivityTracker;
  onRendererInit?() : void;
  onRendererDestroyed?() : void;
  aplCoreMessageHandlerCallback :
      (windowId : string, aplCoreMessageHandler : (message : IAPLCoreMessage) => void) => void;
  backgroundColorOverride? : string;
}

export class WebsocketConnectionWrapper extends APLClient {
  protected client : IClient;
  private supportedExtensions : any;

  constructor(client : IClient) {
    super();
    this.client = client;
  }

  public sendMessage(message : any) {
    this.client.sendMessage(message);
  }
}

export class WindowWebsocketClient extends APLClient {
  protected client : WebsocketConnectionWrapper;
  protected windowId : string;

  constructor(client : WebsocketConnectionWrapper) {
    super();
    this.client = client;
  }

  public setWindowId(id : string) {
    this.windowId = id;
  }

  public sendMessage(message : any) {
    switch (message.type) {
      case 'executeCommands' :
      case 'renderComplete':
      case 'renderStaticDocument' : {
        message.windowId = this.windowId;
        this.client.sendMessage(message);
        break;
      }
      case 'displayMetrics': {
        const metricsEvent = {
          type: 'aplDisplayMetrics',
          windowId: this.windowId,
          payload: message
        };
        this.client.sendMessage(metricsEvent);
        break;
      }
      default : {
        const aplEvent : IAPLEventMessage = {
          type: 'aplEvent',
          windowId: this.windowId,
          payload: message
        };
        this.client.sendMessage(aplEvent);
      }
    }
  }

  public handleMessage(message : IAPLCoreMessage) {
    const unwrapped = message.payload;
    this.onMessage(unwrapped);
  }
}

interface IAPLRendererWindowState {
  windowState : APLRendererWindowState;
}

const DEFAULT_WINDOW_TRANSLATE : string = 'translate(0px,0px)';
type APLWindowFlexType = 'flex-start' | 'flex-end' | 'center';

export class APLRendererWindow extends React.Component<IAPLRendererWindowProps, IAPLRendererWindowState> {
  protected rendererViewportDiv : any;
  protected client : WindowWebsocketClient;
  protected windowConfig : IAPLRendererWindowConfig;
  protected focusManager : IFocusManager;
  protected activityTracker : IActivityTracker;
  protected renderer : APLWSRenderer;
  protected windowFlexAlignItems : APLWindowFlexType;
  protected windowFlexJustifyContent : APLWindowFlexType;
  protected windowInactiveTranslation : string;
  protected windowActiveTransition : string;
  protected windowInactiveTransition : string;
  protected windowActiveTransitionInMS : number;
  protected windowInactiveTransitionInMS : number;
  protected audioPlayer : AVSAudioPlayer;

  private defaultFocusManager : IFocusManager = {
    acquireFocus(
      avsInterface : AVSInterface,
      channelName : ChannelName,
      contentType : ContentType,
      observer : IChannelObserver) : number {
      observer.focusChanged(FocusState.FOREGROUND, undefined);
      return undefined;
    },
    releaseFocus(token : number) {},
    processFocusResponse(token : number, result : any) {},
    processFocusChanged(token : number, focusStateString : string) {},
    reset() {}
  };

  private defaultActivityTracker : IActivityTracker = {
    reportInterrupted() {},
    recordActive(name : string) : number {
      return -1;
    },
    recordInactive(token : number) {},
    reset() {}
  };

  constructor(props : IAPLRendererWindowProps) {
    super(props);
    this.client = new WindowWebsocketClient(props.client);
    this.client.setWindowId(this.props.id);
    this.windowConfig = props.windowConfig;
    this.focusManager = this.props.focusManager || this.defaultFocusManager;
    this.activityTracker = this.props.activityTracker || this.defaultActivityTracker;
    this.rendererViewportDiv = React.createRef();

    const windowPosition = this.windowConfig.windowPosition || 'center';

    this.windowFlexAlignItems = 'center';
    this.windowFlexJustifyContent = 'center';
    this.windowInactiveTranslation = DEFAULT_WINDOW_TRANSLATE;
    this.windowActiveTransitionInMS = OVERLAY_WINDOW_ACTIVE_IN_MS;
    this.windowInactiveTransitionInMS = OVERLAY_WINDOW_INACTIVE_IN_MS;

    let translateX : number = 0;
    let translateY : number = 0;
    let windowOpacityActiveTransitionInMs : number = 0;

    switch (windowPosition) {
      case 'bottom' : {
        this.windowFlexAlignItems = 'flex-end';
        translateY = this.windowConfig.viewport.height;
        break;
      }
      case 'top' : {
        this.windowFlexAlignItems = 'flex-start';
        translateY = -this.windowConfig.viewport.height;
        break;
      }
      case 'right' : {
        this.windowFlexJustifyContent = 'flex-end';
        translateX = this.windowConfig.viewport.width;
        break;
      }
      case 'left' : {
        this.windowFlexJustifyContent = 'flex-start';
        translateX = -this.windowConfig.viewport.width;
        break;
      }
      case 'center' :
      default : {
        this.windowActiveTransitionInMS = WINDOW_TRANSITION_IN_MS;
        this.windowInactiveTransitionInMS = WINDOW_TRANSITION_IN_MS;
        windowOpacityActiveTransitionInMs = WINDOW_TRANSITION_IN_MS;
      }
    }

    this.windowActiveTransition = 'visibility 1s, opacity ' + windowOpacityActiveTransitionInMs + 'ms linear';
    this.windowInactiveTransition = 'visibility 1s, opacity ' + this.windowInactiveTransitionInMS + 'ms linear';
    this.windowInactiveTranslation = 'translate(' + translateX + 'px,' + translateY + 'px)';

    this.state = {
      windowState : APLRendererWindowState.INACTIVE
    };

    const handleAPLCoreMessage = (message : IAPLCoreMessage) => {
      this.client.handleMessage(message);
    };
    props.aplCoreMessageHandlerCallback(this.props.id, handleAPLCoreMessage);
  }

  public render() {
    let windowTransition : string = this.windowActiveTransition;
    let windowTransitionDelay = '0s';
    let windowVisibility : VisibilityProperty = 'visible';
    let windowTranslateTransition = DEFAULT_WINDOW_TRANSLATE;

    if (this.state.windowState === APLRendererWindowState.INACTIVE) {
      windowTransition = this.windowInactiveTransition;
      windowTransitionDelay = this.windowInactiveTransitionInMS + 'ms';
      windowVisibility = 'hidden';
      windowTranslateTransition = this.windowInactiveTranslation;
    }

    return (
        <div className={'aplRendererWindow'}
            tabIndex={0}
            style={{
              opacity: this.state.windowState,
              visibility: windowVisibility,
              alignItems : this.windowFlexAlignItems,
              justifyContent : this.windowFlexJustifyContent,
              transition: windowTransition
            }}>
            <div id={this.props.id}
                className={'aplRendererViewport'}
                style={{
                  transition: 'transform ' + this.windowActiveTransitionInMS + 'ms ease-out ' + windowTransitionDelay,
                  transform: windowTranslateTransition
                }}>
                <div
                  ref={this.rendererViewportDiv}>
                </div>
            </div>
        </div>
    );
  }

  protected createAudioPlayer(eventListener : IAudioEventListener) : AudioPlayer {
    this.audioPlayer = new AVSAudioPlayer(
        AVSInterface.APL,
        ChannelName.DIALOG,
        ContentType.MIXABLE,
        this.focusManager,
        this.activityTracker,
        eventListener);
    return this.audioPlayer;
  }

  protected createRenderer() {
    const rendererElement : HTMLElement = this.rendererViewportDiv.current;

    const environment : IEnvironment = {
      agentName: 'SmartScreenSDK',
      agentVersion: '2.9',
      disallowVideo: this.windowConfig.disallowVideo,
      allowOpenUrl: this.windowConfig.allowOpenUrl,
      animationQuality: this.windowConfig.animationQuality
    };

    const options : IAPLWSOptions = {
      view: rendererElement,
      theme: this.windowConfig.theme,
      viewport : this.windowConfig.viewport,
      mode: this.windowConfig.mode,
      environment,
      audioPlayerFactory: this.createAudioPlayer.bind(this),
      client: this.client,
      videoFactory: new AVSVideoFactory(AVSInterface.APL, ContentType.MIXABLE, this.focusManager, this.activityTracker),
      supportedExtensions: this.windowConfig.supportedExtensions
    } as any;

    this.destroyRenderer();
    this.renderer = APLWSRenderer.create(options);

    this.renderer.init().then(() => {
      console.log(`APL Renderer init resolved with viewport: \
        ${this.renderer.context.getViewportWidth()} x ${this.renderer.context.getViewportHeight()}`);
      rendererElement.style.display = 'flex';
      rendererElement.style.overflow = 'hidden';
      rendererElement.style.width = `${this.renderer.context.getViewportWidth()}px`;
      rendererElement.style.height = `${this.renderer.context.getViewportHeight()}px`;

      // Hack due to fact that APL Web Viewhost doesn't properly support setting background color
      // tslint:disable-next-line:max-line-length
      // https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-document.html#apl_document_background_property
      // tslint:disable-next-line:max-line-length
      // https://github.com/alexa/apl-viewhost-web/blob/master/js/apl-html/src/APLRenderer.ts#L518
      // this.view.style.backgroundColor = backgroundColors[docTheme] will only ever set the bg color to black or white.
      if (this.props.backgroundColorOverride) {
        rendererElement.style.backgroundColor = this.props.backgroundColorOverride;
      }

      // onRendererInit callback
      if (!this.props.clearRenderer && this.props.onRendererInit) {
        this.props.onRendererInit();
      }
      this.setState({
        // Update state once we've rendered
        windowState : this.props.clearRenderer ? APLRendererWindowState.INACTIVE : APLRendererWindowState.ACTIVE
      });

      if (!this.props.clearRenderer) {
        this.displayStateChange(DisplayState.kDisplayStateForeground);
      }

      this.client.sendMessage({
        type: 'renderComplete',
        payload : { }
      });
    });
  }

  protected destroyRenderer() {
    if (this.renderer) {
      this.renderer.destroy();
      (this.renderer as any) = undefined;
      // onRendererDestroyed callback
      if (this.props.onRendererDestroyed) {
        this.props.onRendererDestroyed();
      }
    }
  }

  protected flushAudioPlayer() {
    if (this.audioPlayer) {
      this.audioPlayer.flush();
    }
  }

  protected async setInactive() {
      this.setState({
        windowState : APLRendererWindowState.INACTIVE
      });

      this.displayStateChange(DisplayState.kDisplayStateHidden);

      // Flush audioPlayer for this window before destroying renderer
      this.flushAudioPlayer();

      // Allow window to transition before destroying renderer
      await Timer.delay(this.windowInactiveTransitionInMS);
      // Make sure we didn't become active again before destroying
      if (this.props.clearRenderer) {
        this.destroyRenderer();
      }
  }

  public componentWillReceiveProps(nextProps : IAPLRendererWindowProps) {
    if (nextProps.clearRenderer) {
      this.setInactive();
    }
  }

  public componentWillUpdate() {
    if (this.props.refreshRenderer) {
      this.createRenderer();
    }
  }

  public componentDidUpdate() {
    if (this.props.displayState) {
      this.displayStateChange(this.props.displayState);
    }
  }

  public componentWillUnmount() {
    this.activityTracker.reset();
    this.focusManager.reset();
    this.destroyRenderer();
    this.client.removeAllListeners();
  }

  private displayStateChange (displayState : DisplayState) {
    if (this.renderer) {
      this.renderer.onDisplayStateChange({
        displayState
      });
    }
  }
}
