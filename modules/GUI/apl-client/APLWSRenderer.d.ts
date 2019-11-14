/**
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import APLRenderer, { IAPLOptions } from "./@types/apl-html/lib/index";
import { APLClient } from "./APLClient";
import { APLComponent } from './APLComponent';
export interface IAPLWSOptions extends IAPLOptions {
    /**
     * The client to use to connect to the APLContent server.
     */
    client: APLClient;
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
