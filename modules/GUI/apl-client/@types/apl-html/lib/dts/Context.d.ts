/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
    export class Context extends Deletable {
        public static create(options: any, text: TextMeasure): Context;
        public topComponent(): APL.Component;
        public getTheme(): string;
        public getBackground(): APL.IBackground;
        public getVisualContext(): string;
        public clearPending(): void;
        public isDirty(): boolean;
        public clearDirty(): void;
        public getDirty(): string[];
        public getPendingErrors(): object[];
        public executeCommands(commands: string): Action;
        public scrollToRectInComponent(component: APL.Component, x: number, y: number, width: number, height: number, align: number): void;
        public handleKeyboard(keyType: number, keyboard: APL.Keyboard): void;
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
        public processDataSourceUpdate(payload: string, type: string): boolean;
        public handleDisplayMetrics(dropFrameCount: number): void;
    }
}
