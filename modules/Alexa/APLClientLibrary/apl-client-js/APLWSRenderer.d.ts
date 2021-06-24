/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer, { IAPLOptions } from "./@types/apl-html/lib/index";
import { APLClient } from "./APLClient";
import { APLComponent } from './APLComponent';
export interface IAPLWSOptions extends IAPLOptions {
    /**
     * The client to use to connect to the APLContent server.
     */
    client: APLClient;
    /**
     * List of extension uri's that the renderer will support.
     */
    supportedExtensions?: string[];
}
/**
 * The main renderer. Create a new one with `const renderer = APLWSRenderer.create(options);`
 */
export declare class APLWSRenderer extends APLRenderer<IAPLWSOptions> {
    componentMapping: {
        [id: string]: APLComponent;
    };
    /**
     * Creates a new renderer
     * @param options Options for this instance
     */
    static create(options: IAPLWSOptions): APLWSRenderer;
    init(): Promise<void>;
    destroy(): void;
}
