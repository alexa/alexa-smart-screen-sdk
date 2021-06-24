/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class ExtensionClient {
        public static create(config: APL.RootConfig, uri: string): ExtensionClient;
        public createRegistrationRequest(content: APL.Content): string;
        public processMessage(context: APL.Context | null, message: string): boolean;
        public processCommand(event: APL.Event): string;
    }
}
