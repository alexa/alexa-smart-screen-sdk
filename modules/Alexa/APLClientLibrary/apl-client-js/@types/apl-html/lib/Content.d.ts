/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * Holds all of the documents and data necessary to inflate an APL component hierarchy.
 */
export declare class Content {
    /**
     * Creates an instance of a Content object. a single Content instance
     * can be used with multiple [[APLRenderer]]s.
     * @param doc The main APL document
     */
    static create(doc: string): Content;
    /**
     * APL doc settings.
     * @private
     */
    private settings;
    /**
     * Get Content created from Core.
     */
    getContent(): APL.Content;
    /**
     * Retrieve a set of packages that have been requested.  This method only returns an
     * individual package a single time.  Once it has been called, the "requested" packages
     * are moved internally into a "pending" list of packages.
     * @return The set of packages that should be loaded.
     */
    getRequestedPackages(): Set<APL.ImportRequest>;
    /**
     * Add a requested package to the document.
     * @param request The requested package import structure.
     * @param data Data for the package.
     */
    addPackage(request: APL.ImportRequest, data: string): void;
    /**
     * @return True if this content is in an error state and can't be inflated.
     */
    isError(): boolean;
    /**
     * @return True if this content is complete and ready to be inflated.
     */
    isReady(): boolean;
    /**
     * @return true if this document is waiting for any valid packages to be loaded.
     */
    isWaiting(): boolean;
    /**
     * Add data
     * @param name The name of the data source
     * @param data The raw data source
     */
    addData(name: string, data: string): void;
    /**
     * Get document version specified in the input
     */
    getAPLVersion(): string;
    /**
     * Deletes this obejct and all data associated with it.
     */
    delete(): void;
    /**
     * @return The set of requested custom extensions (a list of URI values)
     */
    getExtensionRequests(): Set<string>;
    /**
     * Retrieve the settings associated with an extension request.
     * @param uri The uri of the extension.
     * @return Map of settings, Object::NULL_OBJECT() if no settings are specified in the document.
     */
    getExtensionSettings(uri: string): object;
    /**
     * get APL settings in APL Doc.
     * @param key
     */
    getAPLSettings(key: string): any;
}
