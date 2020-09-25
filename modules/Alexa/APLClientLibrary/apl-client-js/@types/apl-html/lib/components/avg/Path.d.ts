import { AVG } from './AVG';
import { ILogger } from '../../logging/ILogger';
export declare class Path extends AVG {
    constructor(graphic: APL.GraphicElement, parent: Element, logger: ILogger);
    setAllProperties(): void;
    shouldAssignPathLength(pathLength: number): boolean;
    updateDirty(): void;
    private getStrokeLineCap(graphicLineCap);
    private getStrokeLineJoin(graphicLineJoin);
}
