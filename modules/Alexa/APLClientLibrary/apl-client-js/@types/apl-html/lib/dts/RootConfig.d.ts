/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class RootConfig extends Deletable {
        public static create(environment: any): RootConfig;
        public utcTime(utcTime: number): RootConfig;
        public localTimeAdjustment(localTimeAdjustment: number): RootConfig;
        public localeMethods(localeMethods: any): RootConfig;
        public registerExtensionEventHandler(handler: ExtensionEventHandler): RootConfig;
        public registerExtensionCommand(commandDef: ExtensionCommandDefinition): RootConfig;
        public registerExtensionFilter(commandDef: ExtensionFilterDefinition): RootConfig;
        public registerExtensionEnvironment(uri: string, environment: any): RootConfig;
        public registerExtension(uri: string): RootConfig;
        public liveMap(name: string, obj: any): RootConfig;
        public liveArray(name: string, obj: any): RootConfig;
    }
}
