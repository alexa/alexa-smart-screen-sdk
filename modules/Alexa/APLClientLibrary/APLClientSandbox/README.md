# APL Client Sandbox
The APL Client Sandbox provides a basic implementation of APL Client with a Websocket connection to the APL Viewhost.

## Dependencies
* ASIO (for websocketpp)
* WebsocketPP
* RapidJSON

## Building
`cmake -DWEBSOCKETPP_INCLUDE_DIR=<...> && make`

From the GUI directory:
`npm install`

## Running
Run `APLClientSandbox`

From the GUI directory:
`npm start`

Open a browser and point it to http://127.0.0.1:8000