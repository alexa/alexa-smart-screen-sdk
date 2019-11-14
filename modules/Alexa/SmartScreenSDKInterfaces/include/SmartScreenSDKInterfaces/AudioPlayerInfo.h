/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_AUDIOPLAYERINFO_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_AUDIOPLAYERINFO_H_

#include <AVSCommon/AVS/PlayerActivity.h>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * The @c AudioPlayerInfo contains information that is useful for rendering a PlayerInfo display card.
 * @c AudioPlayerInfo is passed to the observers as a parameter in the @c renderPlayerInfoCard callback.
 */
struct AudioPlayerInfo {
    /**
     * The state of the @c AudioPlayer.  This information is useful for implementing the progress bar
     * in the display card.  It is assumed that the client is responsible for progressing the progress bar
     * when the @c AudioPlayer is in PLAYING state.
     */
    alexaClientSDK::avsCommon::avs::PlayerActivity audioPlayerState;

    /**
     * The offset in millisecond of the media that @c AudioPlayer is handling.  This information is
     * useful for implementation of the progress bar.
     */
    std::chrono::milliseconds offset;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_AUDIOPLAYERINFO_H_