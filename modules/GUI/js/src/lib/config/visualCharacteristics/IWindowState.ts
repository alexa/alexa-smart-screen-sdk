/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

export interface IWindowInstanceConfiguration {
    interactionMode : string;
    sizeConfigurationId : string;
}

export interface IWindowInstance {
    id : string;
    templateId : string;
    token : string;
    configuration : IWindowInstanceConfiguration;
}

export interface IWindowState {
    defaultWindowId : string;
    instances : IWindowInstance[];
}
