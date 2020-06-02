/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { App } from './app';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import './main.css';

window.onload = () => {
    const container = document.createElement('div');
    container.setAttribute('id', 'root');
    document.getElementsByTagName('body')[0].appendChild(container);

    ReactDOM.render(<App/>, document.getElementById('root'));
};
