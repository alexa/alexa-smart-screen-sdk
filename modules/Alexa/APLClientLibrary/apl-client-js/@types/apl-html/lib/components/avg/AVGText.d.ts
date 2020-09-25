import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class AVGText extends AVG {
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    setAllProperties(): void;
    updateDirty(): void;
    private getTextAnchor(graphicTextAnchor);
}
