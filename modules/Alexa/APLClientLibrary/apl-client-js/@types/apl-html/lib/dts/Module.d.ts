/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/// <reference path="./Context.d.ts" />
/// <reference path="./Content.d.ts" />
/// <reference path="./Component.d.ts" />
/// <reference path="./ConfigurationChange.d.ts" />
/// <reference path="./Extension.d.ts" />
/// <reference path="./ExtensionClient.d.ts" />
/// <reference path="./Graphic.d.ts" />
/// <reference path="./GraphicElement.d.ts" />
/// <reference path="./GraphicPattern.d.ts" />
/// <reference path="./Rect.d.ts" />
/// <reference path="./Radii.d.ts" />
/// <reference path="./Action.d.ts" />
/// <reference path="./Event.d.ts" />
/// <reference path="./RootConfig.d.ts" />
/// <reference path="./StyledText.d.ts" />
/// <reference path="./Logger.d.ts" />
/// <reference path="./Metrics.d.ts" />
/// <reference path="./Keyboard.d.ts" />
/// <reference path="./LiveArray.d.ts" />
/// <reference path="./LiveMap.d.ts" />
declare class Exclude<U, V> {
}
declare namespace APL {
    export class Deletable {
        public delete();
    }
    export class Derive<T> extends Deletable {
        public static extend<T>(className: string, def: ClassDef): new () => T;
        public static implement<T>(className: string, def: ClassDef): T;
    }
    export interface Updated {
        id: number;
        props: Array<{
            key: number;
            value: any;
        }>;
    }
    export interface Import {
        id: number;
        name: string;
        version: string;
        source?: string;
    }
    export interface Padding {
        left: number;
        right: number;
        top: number;
        bottom: number;
    }
    export interface IMediaState {
        trackIndex: number;
        trackCount: number;
        currentTime: number;
        duration: number;
        paused: boolean;
        ended: boolean;
    }
    export interface ClassDef {
        __parent?: ClassDef;
        __construct?: Function;
        __destruct?: Function;
        [key: string]: any;
    }
    export class Module {
        public onRuntimeInitialized: () => void;
        public ConfigurationChange: typeof ConfigurationChange;
        public Content: typeof Content;
        public ExtensionCommandDefinition: typeof ExtensionCommandDefinition;
        public ExtensionFilterDefinition: typeof ExtensionFilterDefinition;
        public ExtensionClient: typeof ExtensionClient;
        public ExtensionEventHandler: typeof ExtensionEventHandler;
        public Context: typeof Context;
        public Logger: typeof Logger;
        public RootConfig: typeof RootConfig;
        public Metrics: typeof Metrics;
        public LiveMap: typeof LiveMap;
        public LiveArray: typeof LiveArray;
    }
}
declare var Module: APL.Module;
