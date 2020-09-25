# APL Client Library

The [APL Client Library](https://code.amazon.com/packages/APLClientLibrary/trees/mainline) provides communication between APLViewhostWeb and APLCoreEngine. It's job is making sure that APL Documents are correctly rendered and getting updated based on user's interaction.

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

```
APLClientLibrary is used as a submodule for Alexa Smart Screen SDK, 
therefore it can be built along with Alexa Smart Screen SDK.

Inorder to build APLClientLibrary standalone, follow the instructions below:

cd workspace 
mkdir APLClientProjects
mkdir APLClientLibrary
mkdir build
cd APLClientLibrary
brazil ws use --package APLClientLibrary --branch mainline

Building the package:
cd ../build

cmake ${WORK_AREA}/APLClientProjects/APLClientLibrary \
-DWEBSOCKETPP_INCLUDE_DIR=${WORK_AREA}/sdk_folder/third-party/websocketpp-0.8.1 \
-DPORTAUDIO_LIB_PATH=${WORK_AREA}/portaudio/lib/.libs/libportaudio.a \
-DCMAKE_BUILD_TYPE=DEBUG \
-DAPLCORE_INCLUDE_DIR=${WORK_AREA}/APLCoreLibrary/src/APLCoreEngine/aplcore/include \
-DYOGA_INCLUDE_DIR=${WORK_AREA}/APLCoreLibrary/src/APLCoreEngine/build/yoga-prefix/src/yoga \
-DAPLCORE_RAPIDJSON_INCLUDE_DIR=${WORK_AREA}/APLCoreLibrary/src/APLCoreEngine/build/rapidjson-prefix/src/rapidjson/include/ \
-DAPLCORE_LIB_DIR=${WORK_AREA}/APLCoreLibrary/src/APLCoreEngine/build/aplcore \
-DYOGA_LIB_DIR=${WORK_AREA}/APLCoreLibrary/src/APLCoreEngine/build/lib \
-DAPLCORE_BUILD=${WORK_AREA}/APLCoreLibrary/src/APLCoreEngine/build \
-DAPL_CORE=ON \
-DBUILD_TESTING=OFF \
-DSANDBOX=ON \
-DSTANDALONE=ON

make -j8

```