/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export class Action extends Deletable {
        public resolve();
        public resolveWithArg(arg: number);
        public addTerminateCallback(callback: () => void);
        public then(callback: (action: Action) => void);
        public terminate();
        public isPending(): boolean;
        public isTerminated(): boolean;
        public isResolved(): boolean;
    }
}
