/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { DeviceMode, ScreenMode, IConfigurationChangeOptions } from "./@types/apl-html/lib/index";
/**
 * Create and manage a configuration change payload
 */
export declare class APLConfigurationChange implements APL.ConfigurationChange {
    configurationChangePayload: IConfigurationChangeOptions;
    constructor(payload?: IConfigurationChangeOptions);
    static create(options: IConfigurationChangeOptions): APLConfigurationChange;
    size(width: number, height: number): this;
    width(width: number): this;
    height(height: number): this;
    theme(theme: string): this;
    viewportMode(mode: DeviceMode): this;
    fontScale(fontScale: number): this;
    screenMode(screenMode: ScreenMode): this;
    screenReader(enabled: boolean): this;
    mergeConfigurationChange(other: APLConfigurationChange): void;
    delete(): void;
}
