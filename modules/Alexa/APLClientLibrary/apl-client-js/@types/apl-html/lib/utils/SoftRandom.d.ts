/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * Super simple random implementation based on linear interpolation.
 */
export declare class SoftRandom {
    private static seedValue;
    private static DECIMAL_MASK;
    /**
     * Set random seed. To increased "quality" use of current time is suggested.
     * @param seed Random seed.
     */
    static seed(seed: number): void;
    /**
     * Generate random number in sequence.
     * Number is in range from 0 to 1 with "uniform" distribution.
     */
    static random(): number;
}
