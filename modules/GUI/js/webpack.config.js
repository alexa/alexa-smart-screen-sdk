/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

const HtmlWebpackPlugin = require('html-webpack-plugin');

const path = require('path');
const webpack = require('webpack');

const SRC = path.resolve(__dirname, 'src');
const DEST = path.resolve(__dirname, 'dist');

module.exports = {
    mode: 'development',
    devtool: 'inline-source-map',
    context: SRC,
    entry: {
        'main': './main.tsx'
    },

    output: {
        filename: '[name].bundle.js',
        path: DEST
    },

    resolve: {
        extensions: ['.ts', '.js', '.tsx', '.json', '.css'],
        modules: [
            SRC,
            'node_modules'
        ]
    },

    module: {
        rules: [
            {
                enforce: 'pre',
                test: /\.(ts|tsx)?$/,
                exclude: /node_modules/,
                use: 'tslint-loader'
            },
            {
                test: /\.(ts|tsx)?$/,
                use: 'ts-loader',
                exclude: /node_modules/
            },

            {
                test: /\.css$/,
                use: ['style-loader', 'css-loader']
            },
            {
                test: /\.(png|jpg|gif|wav)$/,
                use: [
                    {
                        loader: 'file-loader',
                        options: {}
                    }
                ]
            }
        ]
    },

    plugins: [
        new HtmlWebpackPlugin()
    ]
};
