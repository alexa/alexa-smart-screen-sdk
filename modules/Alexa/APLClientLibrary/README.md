# APL Client Library

The [APL Client Library](https://github.com/alexa/alexa-smart-screen-sdk/tree/master/modules/Alexa/APLClientLibrary) provides communication between APLViewhostWeb and APLCoreEngine. It's job is making sure that APL Documents are correctly rendered and getting updated based on user's interaction.

The APL Client Library package in this GitHub repo includes:
* The APL Client Library
* A sandbox that enables users to test APL Client Library's functionalities
* apl-client-js: APLViewhostWeb

The APL Client Library depends on the following additional GitHub repos:
* [APL Core Library](https://github.com/alexa/apl-core-library)

## Library Architecture

* **APLCoreTextMeasurement**: Handles text measurement requests from the APL Core Library
* **APLCoreConnectionManager**: Manages the currently rendered APL document
* **APLCoreGUIRenderer**: Handles the creation of APL documents

## Setting Up APL Client Library
## Note: 
* The build script below is an example to build APL 1.5
* Please pull and re-build APLCoreEngine if necessary.
```
APLClientLibrary is used as a submodule for Alexa Smart Screen SDK, 
therefore it can be built along with Alexa Smart Screen SDK.

Inorder to build APLClientLibrary standalone, follow the instructions below:

mkdir workspace
cd workspace 
git clone https://github.com/zaphoyd/websocketpp.git
git clone ssh://git.amazon.com/pkg/APLClientLibrary
git clone https://github.com/alexa/apl-core-library

cd APLClientLibrary
git checkout release-1.5

// build Core
Please follow https://github.com/alexa/apl-core-library#build-prerequisites to build apl core engine

cd ../
mkdir build

Building the package:
cd build

cmake ${WORK_AREA}/APLClientLibrary \
-DWEBSOCKETPP_INCLUDE_DIR=${WORK_AREA}/websocketpp \
-DCMAKE_BUILD_TYPE=DEBUG \
-DAPLCORE_INCLUDE_DIR=${WORK_AREA}/APLCoreEngine/aplcore/include \
-DAPLCORE_BUILD_INCLUDE_DIR=${WORK_AREA}/APLCoreEngine/build/aplcore/include \
-DYOGA_INCLUDE_DIR=${WORK_AREA}/APLCoreEngine/build/yoga-prefix/src/yoga \
-DAPLCORE_RAPIDJSON_INCLUDE_DIR=${WORK_AREA}/APLCoreEngine/build/rapidjson-prefix/src/rapidjson/include/ \
-DAPLCORE_LIB_DIR=${WORK_AREA}/APLCoreEngine/build/aplcore \
-DYOGA_LIB_DIR=${WORK_AREA}/APLCoreEngine/build/lib \
-DAPLCORE_BUILD=${WORK_AREA}/APLCoreEngine/build \
-DAPL_CORE=ON \
-DBUILD_TESTING=OFF \
-DSANDBOX=ON \
-DSTANDALONE=ON

make -j8

```

Running APLClientLibrary Sandbox 
Running Server Side
```
cd $WORK_AREA/build/APLClientSandbox/src
./APLClientSandbox
```
Open another terminal and run the commands below for client side
cd $WORK_AREA/APLClientLibrary/APLClientSandbox/GUI
```
npm install
npm start
```
Copy the address (e.g 127.0.0.1:8000) and run it on a web browser