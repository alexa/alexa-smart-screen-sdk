/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export interface TextMeasure {
        onMeasure(component: APL.Component, width: number, widthMode: number, height: number, heightMode: number): {
            width: number;
            height: number;
        };
        onBaseline(component: APL.Component, width: number, height: number): number;
    }
    export interface IBackground {
        color: string;
        gradient: APL.Image.IGradient | null;
    }
    export type DisplayMetricKind = 'counter' | 'timer';
    export interface DisplayMetric {
        kind: DisplayMetricKind;
        name: string;
        value: number;
    }
    export class Context extends Deletable {
        public static create(options: any, text: TextMeasure, metrics?: APL.Metrics, content?: APL.Content, config?: APL.RootConfig, scalingOptions?: any): Context;
        public topComponent(): APL.Component;
        public getTheme(): string;
        public getBackground(): APL.IBackground;
        public setBackground(background: APL.IBackground): void;
        public getVisualContext(): string;
        public clearPending(): void;
        public isDirty(): boolean;
        public clearDirty(): void;
        public getDirty(): string[];
        public getPendingErrors(): object[];
        public executeCommands(commands: string): Action;
        public invokeExtensionEventHandler(uri: string, name: string, data: string, fastMode: boolean): Action;
        public scrollToRectInComponent(component: APL.Component, x: number, y: number, width: number, height: number, align: number): void;
        public handleKeyboard(keyType: number, keyboard: APL.Keyboard): Promise<boolean>;
        public cancelExecution();
        public hasEvent(): boolean;
        public popEvent(): Event;
        public screenLock(): boolean;
        public currentTime(): number;
        public nextTime(): number;
        public getViewportWidth(): number;
        public getViewportHeight(): number;
        public getScaleFactor(): number;
        public updateTime(currentTime: number, utcTime: number): number;
        public setLocalTimeAdjustment(offset: number): void;
        public updateCursorPosition(x: number, y: number): void;
        public handlePointerEvent(pointerEventType: number, x: number, y: number, pointerId: number, pointerType: number): boolean;
        public processDataSourceUpdate(payload: string, type: string): boolean;
        public handleDisplayMetrics(metrics: APL.DisplayMetric[]): void;
        public configurationChange(configurationChange: APL.ConfigurationChange, metrics?: APL.Metrics, scalingOptions?: any): void;
        public setFocus(direction: number, origin: APL.Rect, targetId: string): void;
        public getFocusableAreas(): Promise<Map<string, APL.Rect>>;
        public getFocused(): Promise<string>;
        public reInflate(): void;
    }
}
