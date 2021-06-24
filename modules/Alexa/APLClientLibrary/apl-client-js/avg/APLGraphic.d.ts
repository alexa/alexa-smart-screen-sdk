/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { GraphicElementData } from "./APLGraphicElement";
export interface GraphicData {
    root: GraphicElementData;
    isValid: boolean;
    intrinsicWidth: number;
    intrinsicHeight: number;
    viewportWidth: number;
    viewportHeight: number;
    dirty: GraphicElementData[];
}
export declare class APLGraphic implements APL.Graphic {
    private root;
    private valid;
    private intrinsicWidth;
    private intrinsicHeight;
    private viewportWidth;
    private viewportHeight;
    private dirty;
    constructor(data: GraphicData);
    private addToDirty(dirty);
    getRoot(): APL.GraphicElement;
    isValid(): boolean;
    getIntrinsicHeight(): number;
    getIntrinsicWidth(): number;
    getViewportWidth(): number;
    getViewportHeight(): number;
    clearDirty(): void;
    getDirty(): {
        [key: number]: APL.GraphicElement;
    };
    delete(): void;
}
