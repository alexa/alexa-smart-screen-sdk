/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
