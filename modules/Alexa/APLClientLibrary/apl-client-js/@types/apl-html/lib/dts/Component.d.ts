/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class Component extends Deletable {
        public getCalculated(): {
            [key: number]: any;
        };
        public getCalculatedByKey<T>(key: number): T;
        public getDirtyProps(): {
            [key: number]: any;
        };
        public getType(): number;
        public getUniqueId(): string;
        public getId(): string;
        public getParent(): Component;
        public update(type: number, value: number): void;
        public updateEditText(type: number, value: string): void;
        public pressed(): void;
        public updateScrollPosition(position: number);
        public updatePagerPosition(position: number);
        public updateMediaState(state: APL.IMediaState, fromEvent: boolean);
        public updateGraphic(json: string);
        public getChildCount(): number;
        public getChildAt(index: number): Component;
        public getDisplayedChildCount(): Promise<number>;
        public getDisplayedChildId(displayIndex: number): Promise<string>;
        public appendChild(child: Component): boolean;
        public insertChild(child: Component, index: number): boolean;
        public remove(): boolean;
        public inflateChild(data: string, index: number): Component;
        public getBoundsInParent(ancestor: Component): APL.Rect;
        public getGlobalBounds(): APL.Rect;
        public ensureLayout(): Promise<void> | void;
        public isCharacterValid(c: string): Promise<boolean>;
        public provenance(): string;
    }
}
