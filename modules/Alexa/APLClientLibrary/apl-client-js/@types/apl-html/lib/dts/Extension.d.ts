/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
declare namespace APL {
    export class ExtensionCommandDefinition {
        public static create(uri: string, name: string): ExtensionCommandDefinition;
        public allowFastMode(allowFastMode: boolean): ExtensionCommandDefinition;
        public requireResolution(requireResolution: boolean): ExtensionCommandDefinition;
        public property(property: string, defValue: any, required: boolean): ExtensionCommandDefinition;
        public arrayProperty(property: string, required: boolean): ExtensionCommandDefinition;
        public getURI(): string;
        public getName(): string;
        public getAllowFastMode(): boolean;
        public getRequireResolution(): boolean;
    }
    export class ExtensionEventHandler {
        public static create(uri: string, name: string): ExtensionEventHandler;
        public getURI(): string;
        public getName(): string;
    }
}
