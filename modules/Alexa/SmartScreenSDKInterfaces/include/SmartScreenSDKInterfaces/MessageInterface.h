/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MESSAGEINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MESSAGEINTERFACE_H

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/// The root GUI message type
const char MSG_TYPE_TAG[] = "type";

/**
 * An interface for rapidjson::Document based messages.
 *
 * All messages have the format:
 * { "type": STRING }
 */
class MessageInterface {
protected:
    rapidjson::Document mDocument;

public:
    /**
     * Constructor
     * @param type The type from this message
     */
    MessageInterface(const std::string& type) : mDocument(rapidjson::kObjectType) {
        auto& alloc = mDocument.GetAllocator();
        mDocument.AddMember(MSG_TYPE_TAG, rapidjson::Value(type.c_str(), alloc).Move(), alloc);
    }
    /**
     * Retrieves the json string representing this message
     * @return json string representation of message
     */
    virtual std::string get() = 0;

    /**
     * Retrieves the @c rapidjson::Value object representation of this message
     * @return @c rapidjson::Value object representation of message
     */
    virtual rapidjson::Value&& getValue() = 0;
};
}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MESSAGEINTERFACE_H
