/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     https://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_LIVEVIEWCONTROLLERCAPABILITYAGENT_INCLUDE_LIVEVIEWCONTROLLERCAPABILITYAGENT_LIVEVIEWCONTROLLERCAPABILITYAGENTOBSERVERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_LIVEVIEWCONTROLLERCAPABILITYAGENT_INCLUDE_LIVEVIEWCONTROLLERCAPABILITYAGENT_LIVEVIEWCONTROLLERCAPABILITYAGENTOBSERVERINTERFACE_H_

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/// Enum for different camera state types
enum class CameraState {
    /// Camera connecting state
    CONNECTING,

    /// Camera connected state
    CONNECTED,

    /// Camera disconnected state
    DISCONNECTED,

    /// Camera error state
    ERROR,

    /// Camera state unknown
    UNKNOWN
};

/**
 * This function converts a CameraState to string
 *
 * @param event the input CameraState
 * @return the string for the input CameraState
 */
inline std::string cameraStateToString(CameraState state) {
    switch (state) {
        case CameraState::CONNECTING:
            return "CONNECTING";
        case CameraState::CONNECTED:
            return "CONNECTED";
        case CameraState::DISCONNECTED:
            return "DISCONNECTED";
        case CameraState::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

/// Enum for different role types
enum class Role {
    /// Camera role
    CAMERA,

    /// Viewer role
    VIEWER,

    /// Unknown rule
    UNKNOWN
};

inline Role roleFromString(const std::string& string) {
    if ("CAMERA" == string) {
        return Role::CAMERA;
    } else if ("VIEWER" == string) {
        return Role::VIEWER;
    } else {
        return Role::UNKNOWN;
    }
}

/// Enum for different two-way talk states
enum class ConcurrentTwoWayTalk {
    /// Two-way talk enabled
    ENABLED,

    /// Two-way talk disabled
    DISABLED,

    /// Two-way talk state unknown
    UNKNOWN
};

inline ConcurrentTwoWayTalk concurrentTwoWayTalkFromString(const std::string& string) {
    if ("ENABLED" == string) {
        return ConcurrentTwoWayTalk::ENABLED;
    } else if ("DISABLED" == string) {
        return ConcurrentTwoWayTalk::DISABLED;
    } else {
        return ConcurrentTwoWayTalk::UNKNOWN;
    }
}

/// Enum for different audio state types (for both speaker and microphone)
enum class AudioState {
    /// Unmuted state
    UNMUTED,

    /// Muted state
    MUTED,

    /// Disabled state
    DISABLED,

    /// Unknown audio state
    UNKNOWN
};

inline AudioState audioStateFromString(const std::string& string) {
    if ("UNMUTED" == string) {
        return AudioState::UNMUTED;
    } else if ("MUTED" == string) {
        return AudioState::MUTED;
    } else if ("DISABLED" == string) {
        return AudioState::DISABLED;
    } else {
        return AudioState::UNKNOWN;
    }
}

class LiveViewControllerCapabilityAgentObserverInterface {
public:
    /**
     * Destructor.
     */
    virtual ~LiveViewControllerCapabilityAgentObserverInterface() = default;

    /**
     * Render camera screen.
     * @param payload the payload to render.
     * @param microphoneAudioState initial state of the camera microphone.
     * @param concurrentTwoWayTalk camera support for two-way talk.
     */
    virtual void renderCamera(
        const std::string& payload,
        AudioState microphoneAudioState,
        ConcurrentTwoWayTalk concurrentTwoWayTalk) = 0;

    /**
     * Receive updates for camera state
     * @param cameraState the incoming camera state
     */
    virtual void onCameraStateChanged(CameraState cameraState) = 0;

    /**
     * This is called for onFirstFrameRendered event
     */
    virtual void onFirstFrameRendered() = 0;

    /**
     * Clear camera related screen.
     */
    virtual void clearCamera() = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_LIVEVIEWCONTROLLERCAPABILITYAGENT_INCLUDE_LIVEVIEWCONTROLLERCAPABILITYAGENT_LIVEVIEWCONTROLLERCAPABILITYAGENTOBSERVERINTERFACE_H_
