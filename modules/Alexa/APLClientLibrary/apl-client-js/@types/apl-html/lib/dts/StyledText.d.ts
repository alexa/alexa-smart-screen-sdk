/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
declare namespace APL {
    export interface TextSpan {
        type: number | 'mark';
        start: number;
        end: number;
    }
    export class StyledText {
        private constructor();
        public text: string;
        public spans: TextSpan[];
    }
}
