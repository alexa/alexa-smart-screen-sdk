/*!
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
        public pressed(): void;
        public updateScrollPosition(position: number);
        public updatePagerPosition(position: number);
        public updateMediaState(state: APL.IMediaState, fromEvent: boolean);
        public updateGraphic(json: string);
        public getChildCount(): number;
        public getChildAt(index: number): Component;
        public appendChild(child: Component): boolean;
        public insertChild(child: Component, index: number): boolean;
        public remove(): boolean;
        public inflateChild(data: string): Component;
        public getBoundsInParent(ancestor: Component): APL.Rect;
        public getGlobalBounds(): APL.Rect;
        public ensureLayout(): Promise<void> | void;
        public provenance(): string;
    }
}
