/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class LiveMap {
        public static create(map?: any): LiveMap;
        public empty(): boolean;
        public clear(): void;
        public get(key: string): any;
        public has(key: string): boolean;
        public set(key: string, value: string): void;
        public update(map: any): void;
        public replace(map: any): void;
        public remove(key: string): boolean;
    }
}
