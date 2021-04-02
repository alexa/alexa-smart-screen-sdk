/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

export class Timer {
    private callback : () => void;
    private timeout : number;
    private intervalId? : number;

    constructor(callback : () => void, timeout : number) {
        this.callback = callback;
        this.timeout = timeout;
    }

    public start () {
        if (this.intervalId) {
            this.stop();
        }
        this.intervalId = window.setInterval(this.callback, this.timeout);
    }

    public stop () {
        clearInterval(this.intervalId);
        this.intervalId = undefined;
    }

    public static delay(ms : number) {
        return new Promise((resolve) => window.setTimeout(resolve, ms));
    }
}
