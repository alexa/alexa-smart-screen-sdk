import { VectorGraphic } from './VectorGraphic';
import { ILogger } from '../../logging/ILogger';
export declare abstract class AVG {
    protected graphic: APL.GraphicElement;
    protected component: VectorGraphic;
    protected parent: Element;
    protected logger: ILogger;
    protected element: Element;
    constructor(graphic: APL.GraphicElement, component: VectorGraphic, parent: Element, logger: ILogger);
    abstract setAllProperties(): any;
    abstract updateDirty(): any;
}
