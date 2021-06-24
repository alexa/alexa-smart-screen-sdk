/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { LogLevel } from './LogLevel';
/**
 * Simple function type to be provided on log initialization to change transport method.
 *
 * @exports
 * @type
 * @function
 *
 * @param level Log level (trace | debug | info | warn | error).
 * @param loggerName Name of the logger. Usually same as component/class name.
 * @param message Actual log message.
 */
export declare type LogTransport = (level: LogLevel, loggerName: string, message: string) => void;
