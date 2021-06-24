/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class LiveArray {
        public static create(array?: any[]): LiveArray;
        public empty(): boolean;
        public clear(): void;
        public size(): number;
        public at(position: number): any;
        public insert(position: number, value: any): boolean;
        public insertRange(position: number, array: any[]): boolean;
        public remove(position: number, count?: number): boolean;
        public update(position: number, value: any): boolean;
        public updateRange(position: number, array: any[]): boolean;
        public push_back(value: any): void;
        public push_backRange(array: any[]): boolean;
    }
}
