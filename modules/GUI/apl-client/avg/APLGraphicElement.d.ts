export declare function toColor(value: string): number;
export declare function toActualSize(value: number): number;
export interface GraphicElementData {
    id: number;
    children: GraphicElementData[];
    props: {
        [key: string]: any;
    };
    type: number;
}
export declare class APLGraphicElement implements APL.GraphicElement {
    private id;
    private type;
    private props;
    private children;
    constructor(data: GraphicElementData);
    getId(): number;
    getChildCount(): number;
    getChildAt(index: number): APL.GraphicElement;
    getValue<T>(key: number): T;
    getDirtyProperties(): number[];
    getType(): number;
    delete(): void;
}
