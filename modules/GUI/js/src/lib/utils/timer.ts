export class Timer {
    private callback : () => void;
    private timeout : number;
    private intervalId? : number;

    constructor(callback : () => void, timeout : number) {
        this.callback = callback;
        this.timeout = timeout;
    }

    public start () {
        if (this.intervalId) {
            this.stop();
        }
        this.intervalId = window.setInterval(this.callback, this.timeout);
    }

    public stop () {
        clearInterval(this.intervalId);
        this.intervalId = undefined;
    }

    public static delay(ms : number) {
        return new Promise((resolve) => window.setTimeout(resolve, ms));
    }
}
