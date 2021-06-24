/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class GraphicPattern extends Deletable {
        public getId(): string;
        public getDescription(): string;
        public getHeight(): number;
        public getWidth(): number;
        public getItemCount(): number;
        public getItemAt(index: number): APL.GraphicElement;
    }
}
