import { ILogger } from '../../logging/ILogger';
export declare abstract class AVG {
    protected graphic: APL.GraphicElement;
    protected parent: Element;
    protected logger: ILogger;
    protected element: Element;
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    abstract setAllProperties(): any;
    abstract updateDirty(): any;
}
