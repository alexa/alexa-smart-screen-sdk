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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCOREVIEWHOSTMESSAGE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCOREVIEWHOSTMESSAGE_H

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace APLClient {

/// The root GUI message type
const char MSG_TYPE_TAG[] = "type";

/// The seqno json key in the message.
const char MSG_SEQNO_TAG[] = "seqno";

/// The payload json key in the message.
const char MSG_PAYLOAD_TAG[] = "payload";

/**
 * The @c AplCoreViewhostMessage base class for messages sent to AplViewHost.
 *
 * { "type": STRING, "seqno": NUMBER, "payload": ANY }
 */
class AplCoreViewhostMessage {
private:
    rapidjson::Document mDocument;

public:
    /**
     * Constructor
     * @param type The type from this message
     */
    AplCoreViewhostMessage(const std::string& type) : mDocument(rapidjson::kObjectType) {
        auto& alloc = mDocument.GetAllocator();
        mDocument.AddMember(MSG_TYPE_TAG, rapidjson::Value(type.c_str(), alloc).Move(), alloc);
    }

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
            MSG_PAYLOAD_TAG,
            rapidjson::Value(payload.c_str(), mDocument.GetAllocator()).Move(),
            mDocument.GetAllocator());
        return *this;
    }

    /**
     * Retrieves the json string representing this message
     * @return json string representation of message
     */
    std::string get() {
        rapidjson::StringBuffer buffer;
        buffer.Clear();
        rapidjson::Writer<
            rapidjson::StringBuffer,
            rapidjson::UTF8<>,
            rapidjson::UTF8<>,
            rapidjson::CrtAllocator,
            rapidjson::kWriteNanAndInfFlag>
            writer(buffer);
        mDocument.Accept(writer);
        return std::string(buffer.GetString(), buffer.GetSize());
    }

    /**
     * Retrieves the rapidjson allocator
     * @return The allocator
     */
    auto alloc() -> decltype(mDocument.GetAllocator()) {
        return mDocument.GetAllocator();
    };
};
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCOREVIEWHOSTMESSAGE_H
