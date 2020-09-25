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

#include "APLClient/AplCoreGuiRenderer.h"
#include "APLClient/Telemetry/NullAplMetricsRecorder.h"
#include "MockAplCoreConnectionManager.h"
#include "MockAplOptionsInterface.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace APLClient {
namespace test {

using namespace ::testing;

static const std::string VIEWPORT_PAYLOAD = "";

static const std::string DOCUMENT_APL_WITH_PACKAGE = "{"
                         "  \"type\": \"APL\","
                         "  \"version\": \"1.0\","
                         "  \"theme\": \"light\","
                         "  \"description\": \"This is a sample APL document\","
                         "  \"import\": ["
                         "       {"
                         "          \"name\":\"alexa-viewport-profiles\","
                         "          \"version\":\"1.0.0\""
                         "       }"
                         " ],"
                         "   \"mainTemplate\": {"
                         "     \"parameters\": ["
                         "       \"payload\""
                         "     ],"
                         "     \"item\": {"
                         "           \"type\": \"Container\","
                         "           \"items\": ["
                         "             {"
                         "               \"type\": \"Text\","
                         "               \"width\": \"100%\","
                         "               \"text\": \"Hello World\","
                         "               \"fontSize\": 50"
                         "             }"
                         "           ]"
                         "     }"
                         "   }"
                         "}";

const std::string SOURCE =
            "https://d2na8397m465mh.cloudfront.net/packages/alexa-viewport-profiles/1.0.0/document.json";

static const std::string TOKEN = "";

static const std::string DATA = "{}";

/// Test harness for @c AplCoreGuiRendererTest class.
class AplCoreGuiRendererTest : public ::testing::Test {

public:
/// Set up the test harness for running a test.
void SetUp() override;

/// Clean up the test harness after running a test.
void TearDown() override;

protected:
    std::shared_ptr<MockAplOptionsInterface> m_mockAplOptions;

    std::shared_ptr<MockAplCoreConnectionManager> m_mockAplCoreConnectionManager;

    std::shared_ptr<AplCoreGuiRenderer> m_aplCoreGuiRenderer;

    std::shared_ptr<Telemetry::AplMetricsRecorderInterface> m_metricsRecorder;

    AplConfigurationPtr m_aplConfiguration;
};

void AplCoreGuiRendererTest::SetUp(){
    m_mockAplOptions = std::make_shared<NiceMock<MockAplOptionsInterface>>();
    m_aplConfiguration = std::make_shared<AplConfiguration>(m_mockAplOptions);
    m_mockAplCoreConnectionManager = std::make_shared<StrictMock<MockAplCoreConnectionManager>>();
    m_aplCoreGuiRenderer = std::make_shared<AplCoreGuiRenderer>(m_aplConfiguration,
                                                            m_mockAplCoreConnectionManager);

    m_metricsRecorder = std::make_shared<Telemetry::NullAplMetricsRecorder>();
    ON_CALL(*m_mockAplOptions, getMetricsRecorder()).WillByDefault(Return(m_metricsRecorder));
}


void AplCoreGuiRendererTest::TearDown(){
    if(m_aplCoreGuiRenderer){
        m_aplCoreGuiRenderer.reset();
    }
}

/**
 * Tests rendering empty document content.
 */
TEST_F(AplCoreGuiRendererTest, RenderEmptyDocumentContent){
    const std::string document = "";

    EXPECT_CALL(*m_mockAplOptions, logMessage(_, _, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(TOKEN, false, "Unable to create content")).Times(1);

    m_aplCoreGuiRenderer->renderDocument(document, DATA, VIEWPORT_PAYLOAD, TOKEN);
}

/**
 * Tests rendering with filled document content.
 */
TEST_F(AplCoreGuiRendererTest, RenderWithDocumentContent){
    const std::string document = "{"
                          "  \"type\": \"APL\","
                          "  \"version\": \"1.0\","
                          "  \"theme\": \"light\","
                          "  \"description\": \"This is a sample APL document\","
                          "  \"import\": [],"
                          "  \"mainTemplate\": {"
                          "    \"parameters\": ["
                          "      \"payload\""
                          "    ],"
                          "    \"item\": {"
                          "          \"type\": \"Container\","
                          "          \"items\": ["
                          "            {"
                          "              \"type\": \"Text\","
                          "              \"width\": \"100%\","
                          "              \"text\": \"Hello World\","
                          "              \"fontSize\": 50"
                          "            }"
                          "          ]"
                          "    }"
                          "  }"
                          "}";

    EXPECT_CALL(*m_mockAplCoreConnectionManager, setSupportedViewports(VIEWPORT_PAYLOAD)).Times(1);
    EXPECT_CALL(*m_mockAplCoreConnectionManager, setContent(_, _)).Times(1);

    m_aplCoreGuiRenderer->renderDocument(document, DATA, VIEWPORT_PAYLOAD, TOKEN);
}

/**
 * Tests a document payload that needs to import packages but package content is empty
 */
TEST_F(AplCoreGuiRendererTest, RenderEmptyPackageContent){

    EXPECT_CALL(*m_mockAplOptions, downloadResource(SOURCE)).Times(1).WillOnce(Return(""));
    EXPECT_CALL(*m_mockAplOptions, logMessage(_, _, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(TOKEN, false, "Unresolved import")).Times(1);
    EXPECT_CALL(*m_mockAplOptions,getMaxNumberOfConcurrentDownloads()).Times(1).WillOnce(Return(5));

    m_aplCoreGuiRenderer->renderDocument(DOCUMENT_APL_WITH_PACKAGE, DATA, VIEWPORT_PAYLOAD, TOKEN);
}

/**
 * Tests a document payload that needs to import packages and successfully adds
 * these packages to the content
 */
TEST_F(AplCoreGuiRendererTest, RenderWithPackageContent){
    const std::string packageContent = "{"
                         " \"type\": \"APL\",   "
                         "      \"version\": \"1.0.0\","
                         "       \"resources\": ["
                         "          {"
                         "            \"description\": \"Definition of density types\","
                         "            \"numbers\":"
                         "             {   "
                         "              \"viewportDensityXLow\": 0,"
                         "              \"viewportDensityLow\": 1,"
                         "              \"viewportDensityNormal\": 2,"
                         "              \"viewportDensityHigh\": 3,"
                         "              \"viewportDensityXHigh\": 4,"
                         "              \"viewportDensityXXHigh\": 5 "
                         "              }"
                         "            }"
                         "       ]"
                         " }";


    EXPECT_CALL(*m_mockAplOptions, downloadResource(SOURCE)).Times(1).WillOnce(Return(packageContent));
    EXPECT_CALL(*m_mockAplOptions, logMessage(_, _, _)).Times(0);
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(0);
    EXPECT_CALL(*m_mockAplCoreConnectionManager, setSupportedViewports(VIEWPORT_PAYLOAD)).Times(1);
    EXPECT_CALL(*m_mockAplCoreConnectionManager, setContent(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions,getMaxNumberOfConcurrentDownloads()).Times(1).WillOnce(Return(5));

    m_aplCoreGuiRenderer->renderDocument(DOCUMENT_APL_WITH_PACKAGE, DATA, VIEWPORT_PAYLOAD, TOKEN);
}


} // namespace test
} // namespace APLClient
