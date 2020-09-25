import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class Group extends AVG {
    children: AVG[];
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    setAllProperties(): void;
    updateDirty(): void;
}
