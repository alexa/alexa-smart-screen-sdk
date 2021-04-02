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

import {
    IDeviceDisplay,
    IDisplayPixelDimensions
} from './visualCharacteristics/IDeviceDisplay';
import {
    IDisplayWindowTemplate,
    IDisplayWindowSize,
    IDisplayWindowDiscreteSize
} from './visualCharacteristics/IDisplayWindow';
import {
    IAppConfig,
    IAPLWindowConfig,
    IDeviceAppConfig,
    AudioInputInitiator,
    IGuiConfig,
    IAPLRendererWindowConfig,
    AplAudioPlayerExtensionUri
} from './IDeviceAppConfig';
import { IInteractionMode } from './visualCharacteristics/IInteractionMode';
import { IVisualCharacteristics } from './visualCharacteristics/IVisualCharacteristics';
import { IVideo } from './visualCharacteristics/IVideo';
import {
    IViewportCharacteristics,
    DeviceMode,
    ILogger
} from 'apl-client';
import {
    IWindowState,
    IWindowInstanceConfiguration,
    IWindowInstance
} from './visualCharacteristics/IWindowState';

const InteractionModeInterface : string = 'Alexa.InteractionMode';
const VideoInterface : string = 'Alexa.Presentation.APL.Video';
const DisplayWindowInterface : string = 'Alexa.Display.Window';
const DisplayInterface : string = 'Alexa.Display';

const AppConfigKey : string = 'appConfig';
const DescriptionKey : string = 'description';
const ModeKey : string = 'mode';
const EmulateDisplayDimensionsKey : string = 'emulateDisplayDimensions';
const ScaleToFillKey : string = 'scaleToFill';
const AudioInputInitiatorKey : string = 'audioInputInitiator';
const WindowsKey : string = 'windows';
const DefaultWindowIdKey : string = 'defaultWindowId';
const DeviceKeysKey : string = 'deviceKeys';
const VisualCharacteristicsKey : string = 'visualCharacteristics';
const InterfaceKey : string = 'interface';
const ConfigurationsKey : string = 'configurations';
const InteractionModesKey : string = 'interactionModes';
const TemplatesKey : string = 'templates';
const DisplayKey : string = 'display';
const VideoKey : string = 'video';

/**
 * Resolves GUI app configuration into IDeviceAppConfig.
 *
 * @param screenWidth width of current browser window (Only used if config not provided)
 * @param screenHeight height of current browser window (Only used if config not provided)
 * @param guiConfigPayload gui config payload received from SDK
 * @returns full device app config of type IDeviceAppConfig
 */
export const resolveDeviceAppConfig = (
    screenWidth : number,
    screenHeight : number,
    guiConfigPayload : any,
    logger : ILogger) : IDeviceAppConfig => {
    const guiConfig : IGuiConfig = resolveGuiConfig(guiConfigPayload);
    const deviceAppConfig = createDeviceAppConfig(
        guiConfig,
        screenWidth,
        screenHeight,
        logger);
    return deviceAppConfig;
};

/**
 * Resolves GUI app configuration reported windows into IWindowState object.
 *
 * @param deviceAppConfig full device app config of type IDeviceAppConfig
 * @returns window state object of type IWindowState
 */
export const resolveDeviceWindowState = (
    deviceAppConfig : IDeviceAppConfig) : IWindowState => {

    let windowState : IWindowState = {
        defaultWindowId : deviceAppConfig.defaultWindowId,
        instances : []
    };

    for (let window of deviceAppConfig.windows) {
        const windowInstanceConfiguration : IWindowInstanceConfiguration = {
            interactionMode : window.interactionMode,
            sizeConfigurationId : window.sizeConfigurationId
        };
        const windowInstance : IWindowInstance = {
            id : window.id,
            templateId : window.templateId,
            token : window.token || null,
            configuration : windowInstanceConfiguration
        };
        windowState.instances.push(windowInstance);
    }

    return windowState;
};

const resolveGuiConfig = (guiConfigPayload : any) : IGuiConfig => {

    const visualCharacteristicsNode : any = guiConfigPayload[VisualCharacteristicsKey];
    const appConfigNode : any = guiConfigPayload[AppConfigKey];

    const visualCharacteristics : IVisualCharacteristics = {
        display : extractDisplay(visualCharacteristicsNode),
        windowTemplates: extractWindowTemplates(visualCharacteristicsNode),
        interactionModes : extractInteractionModes(visualCharacteristicsNode),
        video : extractVideo(visualCharacteristicsNode)
    };

    const appConfig : IAppConfig = {
        description : appConfigNode[DescriptionKey] || '',
        mode : appConfigNode[ModeKey] || 'HUB',
        emulateDisplayDimensions : appConfigNode[EmulateDisplayDimensionsKey] || false,
        scaleToFill : appConfigNode[ScaleToFillKey] || false,
        audioInputInitiator : appConfigNode[AudioInputInitiatorKey] || AudioInputInitiator.TAP,
        windows : appConfigNode[WindowsKey],
        defaultWindowId : appConfigNode[DefaultWindowIdKey],
        deviceKeys : appConfigNode[DeviceKeysKey]
    };

    const guiConfig : IGuiConfig = {
        visualCharacteristics,
        appConfig
    };

    return guiConfig;
};

const extractInteractionModes = (visualCharacteristicsNode : any) : IInteractionMode[] => {
    for (let jsonNode of visualCharacteristicsNode) {
        if (jsonNode[InterfaceKey] === InteractionModeInterface) {
            return jsonNode[ConfigurationsKey][InteractionModesKey];
        }
    }
};

const extractWindowTemplates = (visualCharacteristicsNode : any) : IDisplayWindowTemplate[] => {
    for (let jsonNode of visualCharacteristicsNode) {
        if (jsonNode[InterfaceKey] === DisplayWindowInterface) {
            return jsonNode[ConfigurationsKey][TemplatesKey];
        }
    }
};

const extractDisplay = (visualCharacteristicsNode : any) : IDeviceDisplay => {
    for (let jsonNode of visualCharacteristicsNode) {
        if (jsonNode[InterfaceKey] === DisplayInterface) {
            return jsonNode[ConfigurationsKey][DisplayKey];
        }
    }
};

const extractVideo = (visualCharacteristicsNode : any) : IVideo => {
    for (let jsonNode of visualCharacteristicsNode) {
        if (jsonNode[InterfaceKey] === VideoInterface) {
            return jsonNode[ConfigurationsKey][VideoKey];
        }
    }
};

const createDeviceAppConfig = (
    guiConfig : IGuiConfig,
    screenWidth : number,
    screenHeight : number,
    logger : ILogger) : IDeviceAppConfig => {

    const visualCharacteristics : IVisualCharacteristics = guiConfig.visualCharacteristics;
    const appConfig : IAppConfig = guiConfig.appConfig;

    // Create Device App config from Device config
    const deviceAppConfig : IDeviceAppConfig = Object.assign({}, appConfig) as IDeviceAppConfig;
    deviceAppConfig.rendererWindowConfigs = [];
    deviceAppConfig.display = visualCharacteristics.display;

    for (let windowConfig of appConfig.windows as IAPLWindowConfig[]) {
        let disallowVideo : boolean;
        let mode : DeviceMode = 'HUB';
        let windowTemplate : IDisplayWindowTemplate;

        // Get WindowTemplate from lookup
        for (let wt of visualCharacteristics.windowTemplates) {
            if (wt.id === windowConfig.templateId) {
                windowTemplate = wt;
                break;
            }
        }

        // Get InteractionMode from lookup
        if (windowTemplate &&
            windowTemplate.configuration.interactionModes.includes(windowConfig.interactionMode)) {
            for (let interactionMode of visualCharacteristics.interactionModes) {
                if (interactionMode.id === windowConfig.interactionMode) {
                    disallowVideo = interactionMode.video === 'UNSUPPORTED';
                    mode = interactionMode.uiMode;
                    break;
                }
            }
        }

        // Create Renderer Window config from Window config
        const rendererWindowConfig : IAPLRendererWindowConfig =
            Object.assign({}, windowConfig) as IAPLRendererWindowConfig;
        rendererWindowConfig.mode = mode;
        rendererWindowConfig.disallowVideo = disallowVideo;
        rendererWindowConfig.viewport = createViewportCharacteristicsFromWindowTemplate(
            windowTemplate,
            windowConfig.sizeConfigurationId,
            visualCharacteristics.display,
            screenWidth,
            screenHeight,
            logger
        );

        deviceAppConfig.rendererWindowConfigs.push(rendererWindowConfig);

        // Use overlay windows for renderTemplate directives
        if (deviceAppConfig.renderTemplateWindowId === undefined
            || (windowTemplate && windowTemplate.type === 'OVERLAY')) {
            deviceAppConfig.renderTemplateWindowId = windowConfig.id;
        }

        // use default window config for render player info
        if (rendererWindowConfig.id === deviceAppConfig.defaultWindowId) {
            deviceAppConfig.renderPlayerInfoWindowConfig = rendererWindowConfig;
            // ensure that the player info window supports the Apl AudioPlayer Extension
            let supportedExtensions = deviceAppConfig.renderPlayerInfoWindowConfig.supportedExtensions || [];
            if (!supportedExtensions.includes(AplAudioPlayerExtensionUri)) {
                supportedExtensions.push(AplAudioPlayerExtensionUri);
            }
            deviceAppConfig.renderPlayerInfoWindowConfig.supportedExtensions = supportedExtensions;
        }
    }

    return deviceAppConfig;
};

const createViewportCharacteristicsFromWindowTemplate = (
    windowTemplate : IDisplayWindowTemplate,
    windowSizeId : string,
    deviceDisplay : IDeviceDisplay,
    screenPixelWidth : number,
    screenPixelHeight : number,
    logger : ILogger ) : IViewportCharacteristics => {

    const deviceDisplayPixelDimensions = deviceDisplay.dimensions as IDisplayPixelDimensions;
    let pixelWidth : number = screenPixelWidth;
    let pixelHeight : number = screenPixelHeight;

    if (windowTemplate) {
        let windowSizeConfiguration : IDisplayWindowSize;

        for (let ws of windowTemplate.configuration.sizes) {
            if (ws.id === windowSizeId) {
                windowSizeConfiguration = ws;
                break;
            }
        }

        if (windowSizeConfiguration) {
           if (windowSizeConfiguration.type === 'DISCRETE') {
                const windowSize : IDisplayWindowDiscreteSize =
                    windowSizeConfiguration as IDisplayWindowDiscreteSize;
                pixelWidth = windowSize.value.value.width;
                pixelHeight = windowSize.value.value.height;
            } else {
                 logger.error('CONTINUOUS type is not supported');
            }
        }
    }

    const viewportCharacteristics : IViewportCharacteristics = {
        shape : deviceDisplay.shape,
        dpi : deviceDisplayPixelDimensions.pixelDensity.value,
        isRound : deviceDisplay.shape === 'ROUND',
        width : pixelWidth,
        height : pixelHeight
    };

    return viewportCharacteristics;
};
