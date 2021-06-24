/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { FontStyle } from '../../enums/FontStyle';
import { PropertyKey } from '../../enums/PropertyKey';
import { TextAlign } from '../../enums/TextAlign';
import { TextAlignVertical } from '../../enums/TextAlignVertical';
import { IComponentProperties } from '../Component';
import { Component } from '../Component';
import { MeasureMode } from './MeasureMode';
export interface ITextProperties extends IComponentProperties {
    [PropertyKey.kPropertyColor]: number;
    [PropertyKey.kPropertyFontFamily]: string;
    [PropertyKey.kPropertyFontSize]: number;
    [PropertyKey.kPropertyFontWeight]: string | number;
    [PropertyKey.kPropertyFontStyle]: FontStyle;
    [PropertyKey.kPropertyLetterSpacing]: number;
    [PropertyKey.kPropertyLineHeight]: number;
    [PropertyKey.kPropertyMaxLines]: number;
    [PropertyKey.kPropertyText]: APL.StyledText;
    [PropertyKey.kPropertyTextAlign]: TextAlign;
    [PropertyKey.kPropertyTextAlignVertical]: TextAlignVertical;
}
/**
 * @ignore
 */
export interface ILine {
    lineNumber: number;
    start: number;
    end: number;
}
export declare class Text extends Component<ITextProperties> {
    setDimensions: () => void;
    protected setTextOpacity: () => void;
    private setTextClamping;
    private setText;
    private setFontStyle;
    private setFontWeight;
    private setLetterSpacing;
    private setFontSize;
    private setLineHeight;
    private setFontFamily;
    private setTextAlign;
    private setTextAlignVertical;
    private setColor;
    private setKaraokeColors;
    protected reCreateDOM(): void;
    protected doSplit(): void;
    protected clipMaxLines(): boolean;
    protected addEllipsis(): void;
    protected onPropertiesUpdated(): void;
    protected applyCssShadow: (shadowParams: string) => void;
}
/**
 * Places text on an remote container and is primarily used to
 * to measure text.
 */
export declare class TextMeasurement extends Text {
    protected measurementBox: HTMLDivElement;
    protected $measurementBox: JQuery<HTMLDivElement>;
    protected addComponent(): void;
    protected removeComponent(): void;
    constructor(component: APL.Component, width: number, height: number);
    /**
     * Returns clones of children
     */
    getContents(): Node[];
    onMeasure(width: number, widthMode: MeasureMode, height: number, heightMode: MeasureMode): {
        width: number;
        height: number;
    };
    protected onPropertiesUpdated(): void;
}
