/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class Graphic extends Deletable {
        public getRoot(): APL.GraphicElement;
        public isValid(): boolean;
        public getIntrinsicHeight(): number;
        public getIntrinsicWidth(): number;
        public getViewportWidth(): number;
        public getViewportHeight(): number;
        public clearDirty(): void;
        public getDirty(): {
            [key: number]: APL.GraphicElement;
        };
    }
}
