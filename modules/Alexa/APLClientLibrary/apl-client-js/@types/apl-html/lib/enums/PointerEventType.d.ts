/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
export declare enum PointerEventType {
    /**     * This indicates that a pointer has ended its interaction with the component due to some system level cancellation.     * This is analagous to Android's ACTION_CANCEL, but should *not* be confused with the onCancel APL Touchable     * component event which can occur for other reasons than viewhosts passing along a cancel event     */
    kPointerCancel = 0,
    /**     * This indicates that the pointer has touched down, corresponding to either a mouse button or touch down.  This is     * analogous to HTML's touchstart and onmousedown events, Android's ACTION_DOWN.  This indicates the beginning     * point of a pointer interaction.     */
    kPointerDown = 1,
    /**     * This indicates that the pointer has lifted up, corresponding to either a mouse button or touch.  This is     * analogous to HTML's touchend and onmouseup events, Android's ACTION_UP.  This indicates completion of a     * touch event sequence that started with kPointerDown.     */
    kPointerUp = 2,
    /**     * This indicates that the pointer has moved.  This event will occur for all moving pointers meaning that for     * pointer type devices like mice kPointerMove events occur whenever the pointer moves, whether or not a button     * has been pressed.     */
    kPointerMove = 3,
    /**     * This indicates time update propagated to pointer target. Should not be used directly. In case if there is current     * pointer interaction it will be directed to current pointer target, if no interaction going on it will be     * propagated to the last known target.     */
    kPointerTimeUpdate = 4,
    /**     * This indicates that the pointer target has changed. Should not be used directly, this is issued to the     * last known target internally.     */
    kPointerTargetChanged = 5
}
