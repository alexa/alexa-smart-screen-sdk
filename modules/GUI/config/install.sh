#!/usr/bin/env bash

#
# Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#     http://aws.amazon.com/apache2.0/
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
#

set -o errexit  # Exit the script if any statement fails.
set -o nounset  # Exit the script if any uninitialized variable is used.

PORT_AUDIO_FILE="pa_stable_v190600_20161030.tgz"
PORT_AUDIO_DOWNLOAD_URL="http://www.portaudio.com/archives/$PORT_AUDIO_FILE"
SOUND_CONFIG="$HOME/.asoundrc"

DEVICE_SDK_CLONE_URL=${DEVICE_SDK_CLONE_URL:-'git://github.com/alexa/avs-device-sdk.git'}
DEVICE_SDK_CLONE_TAG=${DEVICE_SDK_CLONE_TAG:-'v1.15'}
SMART_SCREEN_SDK_CLONE_URL=${SMART_SCREEN_SDK_CLONE_URL:-'git@github.com:alexa/alexa-smart-screen-sdk-for-linux.git'}

CURRENT_DIR="$(pwd)"
INSTALL_BASE=${INSTALL_BASE:-"$CURRENT_DIR"}

SOURCE_DIR=${SOURCE_DIR:-'alexaSmartScreenSDK'}
SOURCE_PATH="${INSTALL_BASE}/${SOURCE_DIR}"

BUILD_DIR=${BUILD_FOLDER:-'build'}
BUILD_PATH="${INSTALL_BASE}/${BUILD_DIR}"

DEPENDENCIES_DIR=${DEPENDENCIES_DIR:-'dependencies'}
DEPENDENCIES_PATH="${INSTALL_BASE}/${DEPENDENCIES_DIR}"

DEVICE_SDK_DIR=${DEVICE_SDK_DIR:-'alexaClientSDK'}
DEVICE_SDK_PATH="${INSTALL_BASE}/${DEVICE_SDK_DIR}"

DEVICE_SDK_SOURCE_DIR=${DEVICE_SDK_SOURCE_DIR:-'avs-device-sdk'}
DEVICE_SDK_SOURCE_PATH="${DEVICE_SDK_PATH}/${DEVICE_SDK_SOURCE_DIR}"

DEVICE_SDK_BUILD_DIR=${DEVICE_SDK_BUILD_DIR:-'build'}
DEVICE_SDK_BUILD_PATH="${DEVICE_SDK_PATH}/${DEVICE_SDK_BUILD_DIR}"

DEVICE_SDK_INSTALL_DIR=${DEVICE_SDK_INSTALL_DIR:-'alexaClientSDKInstall'}
DEVICE_SDK_INSTALL_PATH="${DEPENDENCIES_PATH}/${DEVICE_SDK_INSTALL_DIR}"

DEVICE_SDK_THIRD_PARTY_PATH="${DEVICE_SDK_PATH}/third-party"

DB_FOLDER=${DB_FOLDER:-'db'}
DB_PATH="${INSTALL_BASE}/${DB_FOLDER}"
OUTPUT_CONFIG_FILE="${BUILD_PATH}/AlexaClientSDKConfig.json"

APL_CORE_ENGINE_CLONE_URL=${APL_CORE_ENGINE_CLONE_URL:-'https://github.com/alexa/apl-core-library.git'}
APL_CORE_ENGINE_CLONE_TAG=${APL_CORE_ENGINE_CLONE_TAG:-'v1.2'}
APL_CORE_ENGINE_DIR=${APL_CORE_ENGINE_DIR:-'aplCoreEngine'}
APL_CORE_ENGINE_SOURCE_PATH="${INSTALL_BASE}/${APL_CORE_ENGINE_DIR}"
APL_CORE_ENGINE_BUILD_DIR=${APL_CORE_ENGINE_BUILD_DIR:-'build-coreEngine'}
APL_CORE_ENGINE_BUILD_PATH="${DEPENDENCIES_PATH}/${APL_CORE_ENGINE_BUILD_DIR}"

# Default device serial number if nothing is specified
DEVICE_SERIAL_NUMBER="123456"

get_platform() {
  uname_str=`uname -a`
  result=""

  if [[ "$uname_str" ==  "Linux "* ]] && [[ -f /etc/os-release ]]
  then
    sys_id=`cat /etc/os-release | grep "^ID="`
    if [[ "$sys_id" == "ID=raspbian" ]]
    then
      echo "Raspberry pi"
    else
      echo "Linux"
    fi
  elif [[ "$uname_str" ==  "Darwin"* ]]
  then
    echo "Mac"
  else
    echo "Unsupported Platform"
  fi
}

function genConfig {
    ${INSTALL_BASE}/${DEPENDENCIES_DIR}/genConfig.sh \
        ${CONFIG_JSON_FILE} ${DEVICE_SERIAL_NUMBER} ${DB_PATH} \
        ${DEVICE_SDK_SOURCE_PATH} ${OUTPUT_CONFIG_FILE}
}

function show_help {
    echo  'Usage: install.sh <config-json-file> [OPTIONS]'
    echo  'The <config-json-file> can be downloaded from developer portal and must contain the following:'
    echo  '   "clientId": "<OAuth client ID>"'
    echo  '   "productId": "<your product name for device>"'
    echo  ''
    echo  'Optional parameters'
    echo  '  -s <serial-number>  If nothing is provided, the default device serial number is 123456'
    echo  '  -h                  Display this help and exit'
}

function createDirectoriesLayout {
    mkdir -p ${BUILD_PATH}
    mkdir -p ${DEPENDENCIES_PATH}
    mkdir -p ${DEVICE_SDK_PATH}
    mkdir -p ${DEVICE_SDK_INSTALL_PATH}
    mkdir -p ${DEVICE_SDK_BUILD_PATH}
    mkdir -p ${DEVICE_SDK_THIRD_PARTY_PATH}
    mkdir -p ${APL_CORE_ENGINE_BUILD_PATH}
    mkdir -p ${DB_PATH}
}

function installDependencies {
    sudo apt-get update

    # SDK Dependencies
    sudo apt-get -y install git gcc cmake build-essential libsqlite3-dev libcurl4-openssl-dev libssl-dev libfaad-dev libsoup2.4-dev libgcrypt20-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-good libasound2-dev sox gedit vim python3-pip
    pip install flask commentjson

    # SS SDK Dependencies
    sudo apt-get -y install libasio-dev --no-install-recommends

    # NodeJS
    curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
    sudo apt-get -y install nodejs --no-install-recommends
}

function installTools {
    pushd ${INSTALL_BASE}/${DEPENDENCIES_DIR}
        wget https://raw.githubusercontent.com/alexa/avs-device-sdk/${DEVICE_SDK_CLONE_TAG}/tools/Install/genConfig.sh -O genConfig.sh
        chmod +x genConfig.sh
        wget https://github.com/zaphoyd/websocketpp/archive/0.8.1.tar.gz -O websocketpp-0.8.1.tar.gz
        tar -xvzf websocketpp-0.8.1.tar.gz
    popd

    pushd ${DEVICE_SDK_THIRD_PARTY_PATH}
    wget -c $PORT_AUDIO_DOWNLOAD_URL
    tar zxf $PORT_AUDIO_FILE

    pushd portaudio
    [[ "$PLATFORM" == "Mac" ]] && ./configure --disable-mac-universal || ./configure --without-jack
    make
    popd
    popd
}

# Only clone avs-device SDK if folder does NOT exist
function cloneDeviceSDK {
    if [ ! -d $DEVICE_SDK_SOURCE_PATH ]
    then
        pushd ${DEVICE_SDK_PATH}
            git clone --single-branch --branch ${DEVICE_SDK_CLONE_TAG} ${DEVICE_SDK_CLONE_URL} ${DEVICE_SDK_SOURCE_DIR}
        popd
    fi
}

function buildDeviceSDK {
    if [ "$PLATFORM" == "Mac" ]
    then
      export PKG_CONFIG_PATH="/usr/local/opt/libffi/lib/pkgconfig:/usr/local/opt/openssl/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
    fi

    # Install the Device SDK
    pushd ${DEVICE_SDK_BUILD_PATH}
        if [ "$PLATFORM" == "Mac" ]
        then
            cmake -DCMAKE_INSTALL_PREFIX=${DEVICE_SDK_INSTALL_PATH} \
                -DGSTREAMER_MEDIA_PLAYER=ON \
                -DCURL_LIBRARY=/usr/local/opt/curl-openssl/lib/libcurl.dylib \
                -DCURL_INCLUDE_DIR=/usr/local/opt/curl-openssl/include \
                -DCMAKE_BUILD_TYPE=DEBUG \
                -DPORTAUDIO=ON \
                -DPORTAUDIO_LIB_PATH=${DEVICE_SDK_THIRD_PARTY_PATH}/portaudio/lib/.libs/libportaudio.a \
                -DPORTAUDIO_INCLUDE_DIR=${DEVICE_SDK_THIRD_PARTY_PATH}/portaudio/include/ \
                 ${DEVICE_SDK_SOURCE_PATH}
        else
            cmake -DCMAKE_INSTALL_PREFIX=${DEVICE_SDK_INSTALL_PATH} \
                -DGSTREAMER_MEDIA_PLAYER=ON \
                -DCMAKE_BUILD_TYPE=DEBUG \
                -DPORTAUDIO=ON \
                -DPORTAUDIO_LIB_PATH=${DEVICE_SDK_THIRD_PARTY_PATH}/portaudio/lib/.libs/libportaudio.a \
                -DPORTAUDIO_INCLUDE_DIR=${DEVICE_SDK_THIRD_PARTY_PATH}/portaudio/include/ \
                 ${DEVICE_SDK_SOURCE_PATH}
        fi


        make install
    popd
}

# Only clone APL Core Engine if folder does NOT exist
function cloneAplCoreEngine {
    if [ ! -d $APL_CORE_ENGINE_SOURCE_PATH ]
    then
        git clone --single-branch --branch ${APL_CORE_ENGINE_CLONE_TAG} ${APL_CORE_ENGINE_CLONE_URL} ${APL_CORE_ENGINE_SOURCE_PATH}
    fi
}

function buildAplCoreEngine {
    pushd ${APL_CORE_ENGINE_BUILD_PATH}
       cmake $APL_CORE_ENGINE_SOURCE_PATH
       make
    popd
}

# Only clone Smart Screen SDK if folder does NOT exist
function cloneSmartScreenSDK {
    if [ ! -d $SOURCE_PATH ]
    then
        git clone --single-branch ${SMART_SCREEN_SDK_CLONE_URL} ${SOURCE_PATH}
    fi
}

function buildSmartScreenSDK {
	pushd $BUILD_PATH
        cmake -DCMAKE_PREFIX_PATH=${DEVICE_SDK_INSTALL_PATH} \
        -DWEBSOCKETPP_INCLUDE_DIR=${DEPENDENCIES_PATH}/websocketpp-0.8.1 \
        -DDISABLE_WEBSOCKET_SSL=ON \
        -DGSTREAMER_MEDIA_PLAYER=ON \
        -DCMAKE_BUILD_TYPE=DEBUG \
        -DPORTAUDIO=ON -DPORTAUDIO_LIB_PATH=${DEVICE_SDK_THIRD_PARTY_PATH}/portaudio/lib/.libs/libportaudio.a \
        -DPORTAUDIO_INCLUDE_DIR=${DEVICE_SDK_THIRD_PARTY_PATH}/portaudio/include/ \
        -DAPL_CORE=ON \
        -DAPLCORE_INCLUDE_DIR=${APL_CORE_ENGINE_SOURCE_PATH}/aplcore/include \
        -DAPLCORE_LIB_DIR=${APL_CORE_ENGINE_BUILD_PATH}/aplcore \
        -DYOGA_INCLUDE_DIR=${APL_CORE_ENGINE_BUILD_PATH}/yoga-prefix/src/yoga \
        -DYOGA_LIB_DIR=${APL_CORE_ENGINE_BUILD_PATH}/lib \
         ${SOURCE_PATH}
        make
    popd
}

function configureSound() {
  cat << EOF > "$SOUND_CONFIG"
  pcm.!default {
    type asym
     playback.pcm {
       type plug
       slave.pcm "hw:0,0"
     }
     capture.pcm {
       type plug
       slave.pcm "hw:1,0"
     }
  }
EOF
}

if [[ $# -lt 1 ]]; then
    show_help
    exit 1
fi

CONFIG_JSON_FILE=$1
if [ ! -f "$CONFIG_JSON_FILE" ]; then
    echo "Config json file not found!"
    show_help
    exit 1
fi
shift 1

OPTIONS=s:h
while getopts "$OPTIONS" opt ; do
    case $opt in
        s )
            DEVICE_SERIAL_NUMBER="$OPTARG"
            ;;
        h )
            show_help
            exit 1
            ;;
    esac
done

if [[ ! "$DEVICE_SERIAL_NUMBER" =~ [0-9a-zA-Z_]+ ]]; then
   echo 'Device serial number is invalid!'
   exit 1
fi

# The target platform for the build.
PLATFORM=${PLATFORM:-$(get_platform)}


if [ "$PLATFORM" != "Linux" ] && [ "$PLATFORM" != "Raspberry pi" ] && [ "$PLATFORM" != "Mac" ]
then
  echo "The installation script doesn't support current system. (System: $(uname -a))"
  exit 1
fi

echo "################################################################################"
echo "################################################################################"
echo ""
echo ""
echo " Alexa Smart Screen SDK Installation Script - Terms and Agreements"
echo ""
echo ""
echo "The Alexa Smart Screen SDK is dependent on several third-party libraries, , "
echo "environments and/or other software packages that are installed using this script from "
echo "third-party sources (\"External Dependencies\"). These are terms and conditions "
echo "associated with the External Dependencies "
echo "(available at https://github.com/alexa/alexa-smart-screen-sdk-for-linux/wiki/Dependencies) that "
echo "you need to agree to abide by if you choose to install the External Dependencies."
echo ""
echo ""
echo "If you do not agree with every term and condition associated with the External "
echo "Dependencies, enter \"QUIT\" in the command line when prompted by the installer."
echo "Else enter \"AGREE\"."
echo ""
echo ""
echo "################################################################################"
echo "################################################################################"

read input
input=$(echo $input | awk '{print tolower($0)}')
if [ $input == 'quit' ]
then
  exit 1
elif [ $input == 'agree' ]
then
  echo "################################################################################"
  echo "Proceeding with installation"
  echo "################################################################################"
else
  echo "################################################################################"
  echo 'Unknown option'
  echo "################################################################################"
  exit 1
fi

echo "## Creating directory structure"
createDirectoriesLayout

if [ "$PLATFORM" == "Raspberry pi" ]
then
  echo "## Installing dependencies (PI Only)"
  installDependencies
fi

echo "## Install Tools (PortAudio + WebSocket)"
installTools

echo "## Retrieving AVS Device SDK"
cloneDeviceSDK

echo "## Building AVS Device SDK"
buildDeviceSDK

echo "## Retrieving APL Core Engine"
cloneAplCoreEngine

echo "## Building APL Core Engine"
buildAplCoreEngine

echo "## Retrieving Alexa Smart Screen SDK"
cloneSmartScreenSDK

echo "## Building Alexa Smart Screen SDK"
buildSmartScreenSDK

echo "## Generating Configuration"
genConfig

if [ "$PLATFORM" == "Raspberry pi" ]
then
  echo "## Configuring Sound (PI Only)"
  configureSound
fi

echo "## DONE"
