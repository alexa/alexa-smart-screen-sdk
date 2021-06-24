/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class ConfigurationChange extends Deletable {
        public static create(): ConfigurationChange;
        public size(width: number, height: number): ConfigurationChange;
        public theme(theme: string): ConfigurationChange;
        public viewportMode(viewportMode: string): ConfigurationChange;
        public fontScale(scale: number): ConfigurationChange;
        public screenMode(screenMode: string): ConfigurationChange;
        public screenReader(enabled: boolean): ConfigurationChange;
        public mergeConfigurationChange(other: ConfigurationChange): void;
    }
}
