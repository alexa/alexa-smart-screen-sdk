/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from './APLRenderer';
import { Component, IGenericPropType } from './components/Component';
export declare const componentFactory: (renderer: APLRenderer<{
}>, component: APL.Component, parent?: Component<IGenericPropType>, ensureLayout?: boolean, insertAt?: number) => Component<IGenericPropType>;
