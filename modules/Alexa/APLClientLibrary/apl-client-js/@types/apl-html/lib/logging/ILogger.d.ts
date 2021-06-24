/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * Generic logger interface.
 *
 * @exports
 * @interface
 */
export interface ILogger {
    /**
     * Log trace message
     * This will also include a full stack trace
     *
     * @param msg any data to log to the console
     */
    trace(...msg: any[]): void;
    /**
     * Log debug message
     *
     * @param msg any data to log to the console
     */
    debug(...msg: any[]): void;
    /**
     * Log info message
     *
     * @param msg any data to log to the console
     */
    info(...msg: any[]): void;
    /**
     * Log warn message
     *
     * @param msg any data to log to the console
     */
    warn(...msg: any[]): void;
    /**
     * Log error message
     *
     * @param msg any data to log to the console
     */
    error(...msg: any[]): void;
}
