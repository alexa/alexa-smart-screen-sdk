/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import APLRenderer from '../APLRenderer';
import { ActionableComponent } from './ActionableComponent';
import { Component, FactoryFunction, IComponentProperties } from './Component';
import { FontStyle } from '../enums/FontStyle';
import { PropertyKey } from '../enums/PropertyKey';
import { KeyboardType } from '../enums/KeyboardType';
/**
 * @ignore
 */
export interface IEditTextProperties extends IComponentProperties {
    [PropertyKey.kPropertyBorderColor]: number;
    [PropertyKey.kPropertyBorderStrokeWidth]: number;
    [PropertyKey.kPropertyBorderWidth]: number;
    [PropertyKey.kPropertyColor]: number;
    [PropertyKey.kPropertyFontFamily]: string;
    [PropertyKey.kPropertyFontSize]: number;
    [PropertyKey.kPropertyFontStyle]: FontStyle;
    [PropertyKey.kPropertyFontWeight]: string | number;
    [PropertyKey.kPropertyHighlightColor]: number;
    [PropertyKey.kPropertyHint]: string;
    [PropertyKey.kPropertyHintColor]: number;
    [PropertyKey.kPropertyHintStyle]: FontStyle;
    [PropertyKey.kPropertyHintWeight]: string | number;
    [PropertyKey.kPropertyKeyboardType]: KeyboardType;
    [PropertyKey.kPropertyMaxLength]: number;
    [PropertyKey.kPropertySecureInput]: boolean;
    [PropertyKey.kPropertySelectOnFocus]: boolean;
    [PropertyKey.kPropertySize]: number;
    [PropertyKey.kPropertySubmitKeyType]: string;
    [PropertyKey.kPropertyText]: string;
    [PropertyKey.kPropertyValidCharacters]: string;
}
export declare class EditText extends ActionableComponent<IEditTextProperties> {
    formElement: HTMLFormElement;
    inputElement: HTMLInputElement;
    private localFocused;
    private enterPressedDown;
    private isEdge;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    private initEditTextHtmlComponent();
    private setBorderColor;
    private setBorderWidth;
    private setColor;
    private setDisabled;
    private setFontFamily;
    private setFontSize;
    private setFontStyle;
    private setFontWeight;
    private setHighlightColor;
    private setHint;
    private setHintColor;
    private setHintStyle;
    private setHintWeight;
    private setKeyboardType;
    private setMaxLength;
    private setSelectTextOnFocus;
    private setInputSize;
    private setInputText;
    private onFocus;
    private onBlur;
    private onInput;
    private onSubmit;
    private onKeyup;
    private onKeydown;
    private getSectionId;
    protected navigate: (event: any) => void;
    private filterText(text);
    private addRuleToAvailableStyleSheet(selector, style);
}
