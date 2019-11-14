import { AudioPlayer } from './AudioPlayer';
import { IAudioEventListener } from './IAudioEventListener';
import { IAudioContextProvider } from './AudioContextProvider';
export declare class DefaultAudioPlayer extends AudioPlayer {
    protected contextProvider: IAudioContextProvider;
    constructor(eventListener: IAudioEventListener);
    play(id: string): void;
}
