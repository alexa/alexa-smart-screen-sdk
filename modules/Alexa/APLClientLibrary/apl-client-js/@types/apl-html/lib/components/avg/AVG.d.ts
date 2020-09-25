import { ILogger } from '../../logging/ILogger';
import { GradientType } from '../../enums/GradientType';
import { GradientSpreadMethod } from '../../enums/GradientSpreadMethod';
import { GradientUnits } from '../../enums/GradientUnits';
/**
 * @ignore
 */
export interface IAVGGradient {
    type: GradientType;
    colorRange: number[];
    inputRange: number[];
    spreadMethod: GradientSpreadMethod;
    units: GradientUnits;
    x1: number;
    x2: number;
    y1: number;
    y2: number;
    centerX: number;
    centerY: number;
    radius: number;
}
export declare abstract class AVG {
    protected graphic: APL.GraphicElement;
    protected parent: Element;
    protected logger: ILogger;
    static readonly SVG_NS: string;
    protected element: Element;
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    abstract setAllProperties(): any;
    abstract updateDirty(): any;
}
