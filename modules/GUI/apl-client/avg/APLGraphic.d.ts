/**
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { GraphicElementData } from "./APLGraphicElement";
export interface GraphicData {
    root: GraphicElementData;
    isValid: boolean;
    intrinsicWidth: number;
    intrinsicHeight: number;
    viewportWidth: number;
    viewportHeight: number;
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
    getRoot(): APL.GraphicElement;
    isValid(): boolean;
    getIntrinsicHeight(): number;
    getIntrinsicWidth(): number;
    getViewportWidth(): number;
    getViewportHeight(): number;
    clearDirty(): void;
    getDirty(): APL.GraphicElement[];
    delete(): void;
}
