/*!
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
declare namespace APL {
    export class Keyboard {
        private constructor();
        public code: string;
        public key: string;
        public repeat: boolean;
        public altKey: boolean;
        public ctrlKey: boolean;
        public metaKey: boolean;
        public shiftKey: boolean;
    }
}
