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

#ifndef APLCLIENTSANDBOX_INCLUDE_MESSAGE_H
#define APLCLIENTSANDBOX_INCLUDE_MESSAGE_H

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

/// A viewhost message type
const std::string VIEWHOST_MESSAGE_TYPE = "viewhost";

/// A reset message type - used to reset the viewhost
const std::string RESET_MESSAGE_TYPE = "reset";

/// A resource request message type
const std::string RESOURCE_REQUEST_MESSAGE_TYPE = "resourcerequest";

/// The payload json key in the message.
const char MSG_PAYLOAD_TAG[] = "payload";

/// The token json key in the message.
const char MSG_TOKEN_TAG[] = "token";

/// The state json key in the message.
const char MSG_STATE_TAG[] = "state";

/// The type json key in the message
const char MSG_TYPE_TAG[] = "type";

/**
 * All messages have the format:
 * { "type": STRING }
 */
class Message {
protected:
    rapidjson::Document mDocument;

public:
    /**
     * Constructor
     * @param type The type from this message
     */
    Message(const std::string& type) : mDocument(rapidjson::kObjectType) {
        auto& alloc = mDocument.GetAllocator();
        mDocument.AddMember(MSG_TYPE_TAG, rapidjson::Value(type.c_str(), alloc).Move(), alloc);
    }

    /**
     * Add a new member to the json
     * @param name The name of the new member
     * @param value The value of the new member
     * @return this
     */
    Message& addMember(const std::string& name, const std::string& value) {
        mDocument.AddMember(
            rapidjson::Value(name.c_str(), mDocument.GetAllocator()).Move(),
            rapidjson::Value(value.c_str(), mDocument.GetAllocator()).Move(),
            mDocument.GetAllocator());
        return *this;
    }

    /**
     * Add a new member to the json
     * @param name The name of the new member
     * @param value The value of the new member
     * @return this
     */
    Message& addMember(const std::string& name, unsigned value) {
        mDocument.AddMember(
            rapidjson::Value(name.c_str(), mDocument.GetAllocator()).Move(), value, mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the json state for this message
     * @param state The state to send
     * @return this
     */
    Message& setState(const std::string& state) {
        mDocument.AddMember(
            MSG_STATE_TAG, rapidjson::Value(state.c_str(), mDocument.GetAllocator()).Move(), mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the token for this message
     * @param token
     * @return this
     */
    Message& setToken(unsigned token) {
        mDocument.AddMember(MSG_TOKEN_TAG, token, mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the json payload for this message
     * @param payload The payload to send
     * @return this
     */
    Message& setPayload(rapidjson::Value&& payload) {
        mDocument.AddMember(MSG_PAYLOAD_TAG, std::move(payload), mDocument.GetAllocator());
        return *this;
    }

    /**
     * Sets the json payload for this message
     * @param payload The payload to send
     * @return this
     */
    Message& setPayload(const std::string& payload) {
        mDocument.AddMember(
            MSG_PAYLOAD_TAG,
            rapidjson::Value(payload.c_str(), mDocument.GetAllocator()).Move(),
            mDocument.GetAllocator());
        return *this;
    }

    /**
     * Retrieves the rapidjson allocator
     * @return The allocator
     */
    auto alloc() -> decltype(mDocument.GetAllocator()) {
        return mDocument.GetAllocator();
    };

    /**
     * Retrieves the json string representing this message
     * @return json string representation of message
     */
    std::string get() const {
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
     * Retrieves the @c rapidjson::Value object representation of this message
     * @return @c rapidjson::Value object representation of message
     */
    rapidjson::Value&& getValue() {
        return std::move(mDocument);
    };
};

/// A message for the APL Viewhost
class ViewhostMessage : public Message {
public:
    ViewhostMessage(const std::string& message) : Message(VIEWHOST_MESSAGE_TYPE) {
        setPayload(message);
    }
};

/// A message to reset the APL viewhost
class ResetMessage : public Message {
public:
    ResetMessage() : Message(RESET_MESSAGE_TYPE) {
    }
};

/// A message to request a specific resource
class ResourceRequestMessage : public Message {
public:
    ResourceRequestMessage(const std::string& url) : Message(RESOURCE_REQUEST_MESSAGE_TYPE) {
        setPayload(url);
    }
};

#endif  // APLCLIENTSANDBOX_INCLUDE_MESSAGE_H
