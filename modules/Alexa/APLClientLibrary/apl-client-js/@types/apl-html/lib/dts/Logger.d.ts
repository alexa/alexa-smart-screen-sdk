/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export type CoreLogTransport = (level: number, log: string) => void;
    export class Logger extends Deletable {
        public static setLogTransport(transport: CoreLogTransport): void;
    }
}
