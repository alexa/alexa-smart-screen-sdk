/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { GraphicElementData } from "./APLGraphicElement";
export interface GraphicPatternData {
    id: string;
    description: string;
    width: number;
    height: number;
    items: GraphicElementData[];
}
export declare class APLGraphicPattern implements APL.GraphicPattern {
    private id;
    private description;
    private width;
    private height;
    private items;
    constructor(data: GraphicPatternData);
    getId(): string;
    getDescription(): string;
    getHeight(): number;
    getWidth(): number;
    getItemCount(): number;
    getItemAt(index: number): APL.GraphicElement;
    delete(): void;
}
