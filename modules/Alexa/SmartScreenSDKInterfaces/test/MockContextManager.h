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

#ifndef ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKCONTEXTMANAGER_H_
#define ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKCONTEXTMANAGER_H_

#include "AVSCommon/SDKInterfaces/ContextManagerInterface.h"
#include <gmock/gmock.h>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {
namespace test {

/// Mock class that implements the ContextManager.
class MockContextManager : public alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface {
public:
    MOCK_METHOD0(doShutdown, void());
    MOCK_METHOD2(
        setStateProvider,
        void(
            const alexaClientSDK::avsCommon::avs::CapabilityTag& namespaceAndName,
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface> stateProvider));
    MOCK_METHOD4(
        setState,
        alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult(
            const alexaClientSDK::avsCommon::avs::CapabilityTag& namespaceAndName,
            const std::string& jsonState,
            const alexaClientSDK::avsCommon::avs::StateRefreshPolicy& refreshPolicy,
            const unsigned int stateRequestToken));
    MOCK_METHOD3(
        getContext,
        alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken(
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextRequesterInterface>,
            const std::string&,
            const std::chrono::milliseconds&));
    MOCK_METHOD3(
        reportStateChange,
        void(
            const alexaClientSDK::avsCommon::avs::CapabilityTag& capabilityIdentifier,
            const alexaClientSDK::avsCommon::avs::CapabilityState& capabilityState,
            alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause));
    MOCK_METHOD3(
        provideStateResponse,
        void(
            const alexaClientSDK::avsCommon::avs::CapabilityTag& capabilityIdentifier,
            const alexaClientSDK::avsCommon::avs::CapabilityState& capabilityState,
            const unsigned int stateRequestToken));
    MOCK_METHOD3(
        provideStateUnavailableResponse,
        void(
            const alexaClientSDK::avsCommon::avs::CapabilityTag& capabilityIdentifier,
            const unsigned int stateRequestToken,
            bool isEndpointUnreachable));
    MOCK_METHOD1(
        addContextManagerObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerObserverInterface> observer));
    MOCK_METHOD1(
        removeContextManagerObserver,
        void(const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerObserverInterface>&
                 observer));
    MOCK_METHOD2(
        addStateProvider,
        void(
            const alexaClientSDK::avsCommon::avs::CapabilityTag& capabilityIdentifier,
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface> stateProvider));
    MOCK_METHOD1(removeStateProvider, void(const alexaClientSDK::avsCommon::avs::CapabilityTag& capabilityIdentifier));
    MOCK_METHOD3(
        getContextWithoutReportableStateProperties,
        alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken(
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextRequesterInterface> contextRequester,
            const std::string& endpointId,
            const std::chrono::milliseconds& timeout));
};

}  // namespace test
}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKCONTEXTMANAGER_H_
