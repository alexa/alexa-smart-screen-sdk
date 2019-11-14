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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATIONOBSERVERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATIONOBSERVERINTERFACE_H_

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * This @c AlexaPresentationObserverInterface class is used to notify observers when a @c RenderDocument,
 * @c ExecuteCommands directive is received..
 */
class AlexaPresentationObserverInterface {
public:
    /**
     * Destructor
     */
    virtual ~AlexaPresentationObserverInterface() = default;

    /**
     * Used to notify the observer when a Alexa.Presentation.APL.RenderDocument directive is received. Once called, the
     * client should render the document based on the APL specification in the payload in structured JSON format.
     *
     * @note The payload may contain customer sensitive information and should be used with utmost care.
     * Failure to do so may result in exposing or mishandling of customer data.
     *
     * @param jsonPayload The payload of the Alexa.Presentation.APL.RenderDocument directive which follows the APL
     * specification.
     * @param token The APL presentation token associated with this payload.
     * @param windowId The target windowId.
     */
    virtual void renderDocument(
        const std::string& jsonPayload,
        const std::string& token,
        const std::string& windowId) = 0;

    /**
     * Used to notify the observer when the client should clear the APL display card.  Once the card is cleared,
     * the client should call clearDocument().
     */
    virtual void clearDocument() = 0;

    /**
     * Used to notify observer when @c Alexa.Presentation.APL.ExecuteCommands directive has been received.
     *
     * @param jsonPayload The payload of the Alexa.Presentation.APL.ExecuteCommands directive in structured JSON format.
     * @param token Directive token used to bind result processing.
     */
    virtual void executeCommands(const std::string& jsonPayload, const std::string& token) = 0;

    /**
     * Used to notify the observer when a command execution sequence should be interrupted
     */
    virtual void interruptCommandSequence() = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATIONOBSERVERINTERFACE_H_
