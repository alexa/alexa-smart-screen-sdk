class WebsocketConnection {
    constructor(messageCallback) {
        this.connected = false;
        this.callback = messageCallback;
        this.connect();
    }

    connect() {
        const self = this;

        this.socket = new WebSocket("ws://localhost:8080");

        this.socket.onopen = function(e) {
            console.debug("[Websocket] Connection established");
            self.connected = true;
        };

        this.socket.onmessage = function(event) {
            self.callback(JSON.parse(event.data));
            console.debug(`[Websocket] Data received from server: ${event.data}`);
        };

        this.socket.onclose = function(event) {
            self.connected = false;
            if (event.wasClean) {
                console.info(`[Websocket] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
            } else {
                console.warn('[Websocket] Connection died');
            }
            window.setTimeout(self.connect.bind(self), 1000);
        };

        this.socket.onerror = function(error) {
            console.error(`[Websocket] ${error.message}`);
            // reconnect will be handled by onclose
        };
    }

    disconnect() {
        this.socket.close();
    }

    send(message) {
        if (this.connected) {
            if (typeof message != "string") {
                this.socket.send(JSON.stringify(message))
            } else {
                this.socket.send(message);
            }
        } else {
            console.error("[Websocket] No active connection");
        }
    }
}

class Client extends APLClient.APLClient {
    constructor(socket) {
        super();
        this.socket = socket;
    }

    sendMessage(message) {
        this.socket.send({
            type: 'apl',
            payload: message
        });
    }
}

let handleMessage = (data) => {
    switch(data.type) {
        case 'reset':
            resetViewhost();
            break;
        case 'viewhost':
            client.onMessage(JSON.parse(data.payload));
            break;
        case 'resourcerequest':
            handleResourceRequest(data.payload);
            break;
        default:
            console.error("Unknown message type");
            break;
    }
};

let socket = new WebsocketConnection(handleMessage);
let client = new Client(socket);
let renderer;

let load = () => {
    for (const it of ['document', 'data', 'viewports', 'command', 'height', 'width', 'dpi', 'scaling']) {
        if (localStorage.getItem(it)) {
            document.getElementById(it).value = localStorage.getItem(it);
        }
    }
    document.getElementsByClassName('tab')[0].click();
};

let handleResourceRequest = (source) => {
    console.log('retrieving resource', source);
    fetch(source).then((response) => {
        if (!response.ok) {
            socket.send({
                type: 'resourceresponse',
                url: source,
                payload: ''
            });
        } else {
            response.text().then((data) => {
                socket.send({
                    type: 'resourceresponse',
                    url: source,
                    payload: data
                });
            });
        }
    })
};

let resetViewhost = () => {
    console.log('resetViewhost');
    const height = parseInt(document.getElementById('height').value);
    const width = parseInt(document.getElementById('width').value);
    const dpi = parseInt(document.getElementById('dpi').value);
    const scaling = parseFloat(document.getElementById('scaling').value);
    const rendererElement = document.getElementById('renderer');
    const environment = {
        agentName: 'SmartScreenSDK',
        agentVersion: '1.0',
        disallowVideo: false,
        allowOpenUrl: false,
        animationQuality: 'normal'
    };

    const viewport = {
        /** Width in pixels */
        width,
        /** Height in pixels */
        height,
        /** `true` if the screen is round */
        isRound : false,
        /** Dots per inch */
        dpi
    };

    const supportedExtensions=[];
    if(document.getElementById("bacstackExtensions").checked){
        supportedExtensions.push('aplext:backstack:10');
    }
    if(document.getElementById("audioPlayerExtensions").checked){
        supportedExtensions.push('aplext:audioplayer:10');
    }
    if(document.getElementById("musicAlarmsExtensions").checked){
        supportedExtensions.push('aplext:musicalarm:10');
    }

    const options = {
        view: rendererElement,
        theme: 'dark',
        viewport,
        mode: 'TV',
        environment,
        client,
        supportedExtensions
    };

    if (renderer) {
        renderer.destroy();
    }

    renderer = APLClient.APLWSRenderer.create(options);
    renderer.init().then(() => {
        console.log(`APL Renderer init resolved with viewport: \
        ${renderer.context.getViewportWidth()} x ${renderer.context.getViewportHeight()}`);
        rendererElement.style.display = 'flex';
        rendererElement.style.overflow = 'hidden';
        rendererElement.style.width = `${renderer.context.getViewportWidth()}px`;
        rendererElement.style.height = `${renderer.context.getViewportHeight()}px`;
        rendererElement.style.transform = `scale(${scaling})`;
        rendererElement.style.transformOrigin = '0 0';
    });
};

let renderDocument = () => {
    const doc = document.getElementById('document').value;
    const data = document.getElementById('data').value;
    const viewports = document.getElementById('viewports').value;
    const height = parseInt(document.getElementById('height').value);
    const width = parseInt(document.getElementById('width').value);
    const dpi = parseInt(document.getElementById('dpi').value);
    const scaling = parseFloat(document.getElementById('scaling').value);

    localStorage.setItem('document', doc);
    localStorage.setItem('data', data);
    localStorage.setItem('viewports', viewports);
    localStorage.setItem('height', height);
    localStorage.setItem('width', width);
    localStorage.setItem('dpi', dpi);
    localStorage.setItem('scaling', scaling);

    socket.send({
        type: 'renderDocument',
        document: doc,
        data,
        viewports
    });

};

let executeCommand = () => {
    const command = document.getElementById('command').value;
    localStorage.setItem('command', command);
    socket.send({
        type: 'executeCommands',
        command
    })
};

let switchTab = (evt, tabName) => {
    const content = document.getElementsByClassName('content');
    const tabs = document.getElementsByClassName('tab');
    for (const c of content) {
        c.style.display = 'none';
    }
    for (const t of tabs) {
        t.classList.remove('active');
    }
    document.getElementById(tabName).style.display = 'block';
    evt.currentTarget.classList.add('active');
};