import { GraphicData } from "./avg/APLGraphic";
import { Filter, IGradient } from "apl-html";
export declare function toRect(value: [number, number, number, number]): APL.Rect | undefined;
export declare function toTransform(value: [number, number, number, number, number, number]): string;
export declare function toColor(value: string): number;
export declare function toStyledText(value: {
    text: string;
    spans: Array<[number, number, number]>;
}): APL.StyledText;
export declare function toGraphic(value: GraphicData): APL.Graphic | undefined;
export declare function toRadii(value: [number, number, number, number]): APL.Radii;
export declare function toDimension(value: number): number;
export declare function toFilters(value: Filter[]): Filter[];
export declare function toGradient(value: {
    angle: number;
    colorRange: string[];
    inputRange: number[];
    type: number;
}): IGradient;
