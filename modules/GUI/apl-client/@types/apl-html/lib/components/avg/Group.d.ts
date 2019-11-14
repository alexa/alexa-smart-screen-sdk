import { AVG } from './AVG';
import { VectorGraphic } from './VectorGraphic';
import { ILogger } from '../../logging/ILogger';
export declare class Group extends AVG {
    children: AVG[];
    constructor(graphic: APL.GraphicElement, component: VectorGraphic, parent: Element, logger: ILogger);
    setAllProperties(): void;
    updateDirty(): void;
}
