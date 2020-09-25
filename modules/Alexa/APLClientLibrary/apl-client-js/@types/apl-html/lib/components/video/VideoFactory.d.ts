import APLRenderer from '../../APLRenderer';
import { Component, FactoryFunction } from '../Component';
import { AbstractVideoComponent } from './AbstractVideoComponent';
import { IVideoFactory } from './IVideoFactory';
export declare class VideoFactory implements IVideoFactory {
    create(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component): AbstractVideoComponent;
}
