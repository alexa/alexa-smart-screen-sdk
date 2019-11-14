/**
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
export declare class Content {
    static create(doc: string): Content;
    content: APL.Content;
    private constructor();
    getRequestedPackages(): Set<APL.ImportRequest>;
    addPackage(request: APL.ImportRequest, data: string): void;
    isError(): boolean;
    isReady(): boolean;
    isWaiting(): boolean;
    addData(name: string, data: string): void;
    delete(): void;
}
