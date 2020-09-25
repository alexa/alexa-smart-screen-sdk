import APLRenderer from '../../APLRenderer';
import { Component, FactoryFunction } from '../Component';
import { AbstractVideoComponent } from './AbstractVideoComponent';
export interface IVideoFactory {
    create(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component): AbstractVideoComponent;
}
