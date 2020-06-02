/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { DeviceMode, IViewportCharacteristics, AnimationQuality } from 'apl-client';
import { IVisualCharacteristics } from './visualCharacteristics/IVisualCharacteristics';
import { IDeviceDisplay } from './visualCharacteristics/IDeviceDisplay';

/**
 * Copy of alexaClientSDK/capabilityAgents/aip/Initiator.
 * Enumerates the different initiators supported by AVS.
 *
 * @enum
 * @exports
 */
export enum AudioInputInitiator {
    /// Recognize event was initiated by a press-and-hold action.
    PRESS_AND_HOLD = 'PRESS_AND_HOLD',
    /// Recognize event was initiated by a tap-to-talk action.
    TAP = 'TAP',
    /// Recognize event was initiated by a wakeword action.
    WAKEWORD = 'WAKEWORD'
}

/**
 * Type which designates the on-screeen position of the window.
 */
export type APLRendererWindowPositionType = 'center' | 'top' | 'bottom' | 'left' | 'right';

 /**
  * Interface for modeling Device keys
  */
export interface IDeviceKey {
    code : string;
    keyCode : number;
    key : string;
}

/**
 * Device keys used for primary interactions
 */
export interface IDeviceKeys {
    /** Key to initiate audio input */
    talkKey : IDeviceKey;
    /** Key to exit */
    exitKey : IDeviceKey;
    /** Key to go back */
    backKey : IDeviceKey;
    /** Key to toggle captions */
    toggleCaptionsKey : IDeviceKey;
}

/**
 * Interface for GUI configuration reported from SDK
 */
export interface IGuiConfig {
    /** DCF Visual Characteristics */
    visualCharacteristics : IVisualCharacteristics;
    /** GUI App Config */
    appConfig : IAppConfig;
}

/**
 * Interface for defining available windows as configured from DCF window templates
 */
export interface IAPLWindowConfig {
    /** Unique Id used for window targeting - reported in window state */
    id : string;
    /** Id of window template from device characteristic templates - reported in window state */
    templateId : string;
    /** Window template size configuration index to use - reported in window state */
    sizeConfigurationId : string;
    /** Window template interaction mode to use - reported in window state */
    interactionMode : string;
    /** Device-named theme parameter for APL document. Usually 'light' or 'dark' */
    theme? : string;
    /** `true` if OpenURL command is supported. Defaults to `false` */
    allowOpenUrl? : boolean;
    /** Level of animation quality. Defaults to `AnimationQuality.kAnimationQualityNormal` */
    animationQuality? : AnimationQuality;
    /** Position of window on display.  Defaults to `center` */
    windowPosition? : APLRendererWindowPositionType;
    /** The token identifying the content currently occupying the window */
    token : string;
}

/**
 * Interface for defining full GUI App config
 */
export interface IAppConfig {
    /** Device Description */
    description : string;
    /** Explicilty set app root div to display dimensions */
    emulateDisplayDimensions? : boolean;
    /** HACK param to css zoom app root div based on container dimensions */
    scaleToFill? : boolean;
    /** Device mode. Used for determining global CX behaviors like voice chrome */
    mode : DeviceMode;
    /** Initiator used for audio input */
    audioInputInitiator : AudioInputInitiator;
    /** Z-ordered collection of window configs */
    windows : IAPLWindowConfig[];
    /** The default window id to target if none is provided by content response */
    defaultWindowId : string;
    /** Device keys used for primary interactions */
    deviceKeys : IDeviceKeys;
}

/**
 * Resolved Configuration model for APLRendererWindow Component
 */
export interface IAPLRendererWindowConfig extends IAPLWindowConfig {
    /** Window device mode. Mode of the window itself. If none provided "HUB" is used. */
    mode? : DeviceMode;
    /** Physical viewport characteristics */
    viewport : IViewportCharacteristics;
    /** `true` if video is not supported. Defaults to `false` */
    disallowVideo? : boolean;
}

/**
 * Resolved Configuration model for Device App with APLRendererWindows
 */
export interface IDeviceAppConfig extends IAppConfig {
    /** Physical Device Display Characteristics */
    display : IDeviceDisplay;
    /** Target window id for render template directives */
    renderTemplateWindowId : string;
    /** Unique config for PlayerInfo APLRendererWindow */
    renderPlayerInfoWindowConfig : IAPLRendererWindowConfig;
    /** Z-Ordered array of configs for APLRendererWindows */
    rendererWindowConfigs : IAPLRendererWindowConfig[];
}
