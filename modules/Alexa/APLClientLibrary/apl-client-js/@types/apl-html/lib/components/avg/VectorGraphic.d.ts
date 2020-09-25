import APLRenderer from '../../APLRenderer';
import { PropertyKey } from '../../enums/PropertyKey';
import { VectorGraphicAlign } from '../../enums/VectorGraphicAlign';
import { VectorGraphicScale } from '../../enums/VectorGraphicScale';
import { Component, FactoryFunction, IComponentProperties } from '../Component';
import { AVG } from './AVG';
import { TouchableComponent } from '../TouchableComponent';
export { AVG } from './AVG';
export { Group } from './Group';
export { Path } from './Path';
export { AVGText } from './AVGText';
export { Patterns } from './Patterns';
export interface IVectorGraphicProperties extends IComponentProperties {
    [PropertyKey.kPropertyAlign]: VectorGraphicAlign;
    [PropertyKey.kPropertyGraphic]: APL.Graphic;
    [PropertyKey.kPropertyMediaBounds]: APL.Rect;
    [PropertyKey.kPropertyScale]: VectorGraphicScale;
    [PropertyKey.kPropertySource]: string;
}
/**
 * @ignore
 */
export declare class VectorGraphic extends TouchableComponent<IVectorGraphicProperties> {
    static readonly SVG_NS: string;
    elements: AVG[];
    private graphic;
    private svg;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    setProperties(props: IVectorGraphicProperties): Promise<void>;
}
