/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
/// <reference path="./Context.d.ts" />
/// <reference path="./Content.d.ts" />
/// <reference path="./Component.d.ts" />
/// <reference path="./Graphic.d.ts" />
/// <reference path="./GraphicElement.d.ts" />
/// <reference path="./Rect.d.ts" />
/// <reference path="./Radii.d.ts" />
/// <reference path="./Action.d.ts" />
/// <reference path="./Event.d.ts" />
/// <reference path="./StyledText.d.ts" />
/// <reference path="./Logger.d.ts" />
/// <reference path="./Keyboard.d.ts" />
// TODO -> remove when upgrading to TS 3.0
// https://issues.labcollab.net/browse/ARC-867
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
        public Content: typeof Content;
        public Context: typeof Context;
        public Logger: typeof Logger;
    }
}
declare var Module: APL.Module;
