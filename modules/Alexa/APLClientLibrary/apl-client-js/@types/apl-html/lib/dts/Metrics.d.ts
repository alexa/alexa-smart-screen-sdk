/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class Metrics extends Deletable {
        public static create(): Metrics;
        public size(width: number, height: number): Metrics;
        public dpi(dpi: number): Metrics;
        public theme(theme: string): Metrics;
        public shape(shape: string): Metrics;
        public mode(mode: string): Metrics;
    }
}
