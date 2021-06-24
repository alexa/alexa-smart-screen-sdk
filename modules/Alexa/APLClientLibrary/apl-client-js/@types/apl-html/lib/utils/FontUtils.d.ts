/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
export declare class FontUtils {
    private static logger;
    private static readonly AMAZON_EMBER_DISPLAY;
    private static readonly BOOKERLY;
    private static readonly FONT_ALIAS_MAPPING;
    private static readonly FONT_STYLE_MAPPING;
    private static initialized;
    private static initializationCallback;
    static initialize(): Promise<void>;
    static getFont(fontFamily: string): string;
    static getFontStyle(fontStyle: number): string;
}
