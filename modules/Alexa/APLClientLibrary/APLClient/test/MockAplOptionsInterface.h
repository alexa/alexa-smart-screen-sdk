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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace APLClient {
namespace test {

class MockAplOptionsInterface : public AplOptionsInterface {
public:
    MOCK_METHOD2(sendMessage, void(const std::string& token, const std::string& payload));
    MOCK_METHOD1(resetViewhost, void(const std::string& token));
    MOCK_METHOD1(downloadResource, std::string(const std::string& source));
    MOCK_METHOD0(getTimezoneOffset, std::chrono::milliseconds());
    MOCK_METHOD2(onActivityStarted, void(const std::string& token, const std::string& source));
    MOCK_METHOD2(onActivityEnded, void(const std::string& token, const std::string& source));
    MOCK_METHOD2(onSendEvent, void(const std::string& token, const std::string& event));
    MOCK_METHOD2(onCommandExecutionComplete, void(const std::string& token, bool result));
    MOCK_METHOD3(onRenderDocumentComplete, void(const std::string& token, bool result, const std::string& error));
    MOCK_METHOD3(
        onVisualContextAvailable,
        void(const std::string& token, unsigned int stateRequestToken, const std::string& context));
    MOCK_METHOD2(onSetDocumentIdleTimeout, void(const std::string& token, const std::chrono::milliseconds& timeout));
    MOCK_METHOD2(onRenderingEvent, void(const std::string& token, AplRenderingEvent event));
    MOCK_METHOD1(onFinish, void(const std::string& token));
    MOCK_METHOD3(
        onDataSourceFetchRequestEvent,
        void(const std::string& token, const std::string& type, const std::string& payload));
    MOCK_METHOD2(onRuntimeErrorEvent, void(const std::string& token, const std::string& payload));
    MOCK_METHOD3(logMessage, void(LogLevel level, const std::string& source, const std::string& message));
    MOCK_METHOD0(getMaxNumberOfConcurrentDownloads, int());
    MOCK_METHOD0(getMetricsRecorder, Telemetry::AplMetricsRecorderInterfacePtr());
    MOCK_METHOD1(setMetricsRecorder, void(Telemetry::AplMetricsRecorderInterfacePtr));
    MOCK_METHOD7(
        onExtensionEvent,
        void(
            const std::string& aplToken,
            const std::string& uri,
            const std::string& name,
            const std::string& source,
            const std::string& params,
            unsigned int event,
            std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback));
};

}  // namespace test
}  // namespace APLClient
