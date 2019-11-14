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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_APLCOREVIEWHOSTMESSAGE_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_APLCOREVIEWHOSTMESSAGE_H

#include <string>

#include "Message.h"

/// The seqno json key in the message.
const char MSG_SEQNO_TAG[] = "seqno";

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace messages {

/**
 * The @c AplCoreViewhostMessage base class for @c Messages sent to AplViewHost.
 * 
 * { "type": STRING, "seqno": NUMBER, "payload": ANY }
 */
class AplCoreViewhostMessage : public Message {
public:
    /**
     * Constructor
     * @param type The type from this message
     */
    AplCoreViewhostMessage(const std::string& type) : Message(type) {}

    /**
     * Sets the sequence number for this message
     * @param sequenceNumber
     * @return this
     */
    AplCoreViewhostMessage& setSequenceNumber(unsigned sequenceNumber) {
        mDocument.AddMember(MSG_SEQNO_TAG, sequenceNumber, mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the json payload for this message
     * @param payload The payload to send
     * @return this
     */
    AplCoreViewhostMessage& setPayload(rapidjson::Value&& payload) {
        mDocument.AddMember(MSG_PAYLOAD_TAG, std::move(payload), mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the json payload for this message
     * @param payload The payload to send
     * @return this
     */
    AplCoreViewhostMessage& setPayload(const std::string& payload) {
        mDocument.AddMember(
            MSG_PAYLOAD_TAG, rapidjson::Value(payload.c_str(), mDocument.GetAllocator()).Move(), mDocument.GetAllocator());
        return *this;
    }
};
}  // namespace messages
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_MESSAGES_APLCOREVIEWHOSTMESSAGE_H
