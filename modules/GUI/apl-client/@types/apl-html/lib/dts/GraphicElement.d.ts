/*!
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
declare namespace APL {
    export class GraphicElement extends Deletable {
        public getId(): number;
        public getChildCount(): number;
        public getChildAt(index: number): APL.GraphicElement;
        public getValue<T>(key: number): T;
        public getDirtyProperties(): number[];
        public getType(): number;
    }
}
