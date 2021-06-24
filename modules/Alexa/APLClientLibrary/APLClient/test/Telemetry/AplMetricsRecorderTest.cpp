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

#include "APLClient/Telemetry/AplMetricsRecorder.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

namespace APLClient {
namespace Telemetry {
namespace test {

class MockAplMetricsSinkInterface : public AplMetricsSinkInterface {
public:
    MOCK_METHOD3(reportTimer, void(const std::map<std::string, std::string> &,
                                   const std::string&,
                                   const std::chrono::nanoseconds&));
    MOCK_METHOD3(reportCounter, void(const std::map<std::string, std::string> &,
                                     const std::string&,
                                     uint64_t));
};

class AplMetricsRecorderTest : public ::testing::Test {
public:
    /// Set up the test harness for running a test.
    void SetUp() override;

protected:
    std::shared_ptr<MockAplMetricsSinkInterface> m_mockSink;
    std::shared_ptr<AplMetricsRecorder> m_metricsRecorder;
    AplMetricsRecorderInterface::DocumentId m_document;
};

void AplMetricsRecorderTest::SetUp() {
    m_mockSink = std::make_shared<StrictMock<MockAplMetricsSinkInterface>>();
    m_metricsRecorder = std::dynamic_pointer_cast<AplMetricsRecorder>(AplMetricsRecorder::create(m_mockSink));
    m_document = m_metricsRecorder->registerDocument();
}

TEST_F(AplMetricsRecorderTest, RegistersNewDocuments) {
    ASSERT_GT(m_document, AplMetricsRecorderInterface::UNKNOWN_DOCUMENT);
    ASSERT_GT(m_document, AplMetricsRecorderInterface::CURRENT_DOCUMENT);
    ASSERT_GT(m_document, AplMetricsRecorderInterface::LATEST_DOCUMENT);
}

TEST_F(AplMetricsRecorderTest, ProcessesRenderingStart) {
    ASSERT_EQ(AplMetricsRecorderInterface::UNKNOWN_DOCUMENT, m_metricsRecorder->latestDocument());
    auto id = m_metricsRecorder->registerDocument();
    m_metricsRecorder->onRenderingStarted(id);
    ASSERT_EQ(id, m_metricsRecorder->latestDocument());
}

TEST_F(AplMetricsRecorderTest, ProcessesRenderingEnd) {
    ASSERT_EQ(AplMetricsRecorderInterface::UNKNOWN_DOCUMENT, m_metricsRecorder->currentDisplayedDocument());
    auto id = m_metricsRecorder->registerDocument();
    m_metricsRecorder->onRenderingStarted(id);
    m_metricsRecorder->onRenderingEnded(id);
    ASSERT_EQ(id, m_metricsRecorder->currentDisplayedDocument());
}

TEST_F(AplMetricsRecorderTest, CreatesNamedTimers) {
    auto timer = m_metricsRecorder->createTimer(m_document, "MyTimer");
    std::chrono::steady_clock::time_point startTime(std::chrono::nanoseconds(10000));
    timer->startedAt(startTime);

    std::chrono::steady_clock::time_point endTime(std::chrono::nanoseconds(15000));
    timer->stoppedAt(endTime);

    EXPECT_CALL(*m_mockSink, reportTimer(IsEmpty(), Eq("MyTimer"), Eq(std::chrono::nanoseconds(5000))))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, CreatesSegmentTimers) {
    auto timer = m_metricsRecorder->createTimer(m_document, AplRenderingSegment::kContentCreation);
    std::chrono::steady_clock::time_point startTime(std::chrono::nanoseconds(10000));
    timer->startedAt(startTime);

    std::chrono::steady_clock::time_point endTime(std::chrono::nanoseconds(15000));
    timer->stoppedAt(endTime);

    EXPECT_CALL(*m_mockSink, reportTimer(IsEmpty(), Eq("APL-Web.Content.create"), Eq(std::chrono::nanoseconds(5000))))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, InfersRenderingProgressFromSegments) {
    ASSERT_EQ(AplMetricsRecorderInterface::UNKNOWN_DOCUMENT, m_metricsRecorder->latestDocument());
    ASSERT_EQ(AplMetricsRecorderInterface::UNKNOWN_DOCUMENT, m_metricsRecorder->currentDisplayedDocument());

    auto timer = m_metricsRecorder->createTimer(m_document, AplRenderingSegment::kRenderDocument);
    timer->start();

    EXPECT_EQ(m_document, m_metricsRecorder->latestDocument());
    EXPECT_EQ(AplMetricsRecorderInterface::UNKNOWN_DOCUMENT, m_metricsRecorder->currentDisplayedDocument());

    EXPECT_CALL(*m_mockSink, reportTimer(IsEmpty(), Eq("SmartScreenSDK.renderDocument"), _))
        .Times(1);

    timer->stop();

    EXPECT_EQ(m_document, m_metricsRecorder->currentDisplayedDocument());
    EXPECT_EQ(m_document, m_metricsRecorder->latestDocument());
}

TEST_F(AplMetricsRecorderTest, ReportsTimerFailures) {
    auto timer = m_metricsRecorder->createTimer(m_document, "MyTimer");
    timer->start();
    timer->fail();

    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("MyTimer.fail"), Eq(1UL)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, ReportsZeroTimerFailuresIfRequested) {
    auto timer = m_metricsRecorder->createTimer(m_document, "MyTimer", true);
    timer->start();
    timer->stop();

    EXPECT_CALL(*m_mockSink, reportTimer(IsEmpty(), Eq("MyTimer"), _))
        .Times(1);
    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("MyTimer.fail"), Eq(0UL)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, ReportsTimerMetadata) {
    const std::chrono::nanoseconds duration(5000);
    auto timer = m_metricsRecorder->createTimer(m_document, "MyTimer");
    timer->elapsed(duration);
    m_metricsRecorder->addMetadata(m_document, "myKey", "myValue");

    EXPECT_CALL(*m_mockSink, reportTimer(ElementsAre(Pair("myKey", "myValue")), Eq("MyTimer"), Eq(duration)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, CreatesNamedCounters) {
    auto counter = m_metricsRecorder->createCounter(m_document, "MyCounter");

    counter->incrementBy(42);

    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("MyCounter"), Eq(42UL)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, CreatesSegmentCounters) {
    auto counter = m_metricsRecorder->createCounter(m_document, AplRenderingSegment::kTextMeasure);

    counter->incrementBy(42);

    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("APL-Web.RootContext.measureCount"), Eq(42UL)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, ReportsZeroCountersIfRequested) {
    auto counter = m_metricsRecorder->createCounter(m_document, "MyCounter", true);

    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("MyCounter"), Eq(0UL)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, SkipsReportingZeroCountersIfRequested) {
    auto counter = m_metricsRecorder->createCounter(m_document, "MyCounter", false);

    // strict mock will fail if any counter gets reported
    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, ReportsCounterMetadata) {
    auto counter = m_metricsRecorder->createCounter(m_document, "MyCounter");
    counter->increment();
    m_metricsRecorder->addMetadata(m_document, "myKey", "myValue");

    EXPECT_CALL(*m_mockSink, reportCounter(ElementsAre(Pair("myKey", "myValue")), Eq("MyCounter"), Eq(1UL)))
        .Times(1);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, ReportsTimersAndCountersOnlyOnce) {
    const std::chrono::nanoseconds duration(5000);
    auto timer = m_metricsRecorder->createTimer(m_document, "MyTimer");
    timer->elapsed(duration);

    auto counter = m_metricsRecorder->createCounter(m_document, "MyCounter");
    counter->increment();

    EXPECT_CALL(*m_mockSink, reportTimer(IsEmpty(), Eq("MyTimer"), Eq(duration)))
        .Times(1);
    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("MyCounter"), Eq(1UL)))
        .Times(1);
    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), Eq("MyNewCounter"), Eq(2UL)))
        .Times(1);

    m_metricsRecorder->flush();

    auto newCounter = m_metricsRecorder->createCounter(m_document, "MyNewCounter");
    newCounter->incrementBy(2);

    m_metricsRecorder->flush();
}

TEST_F(AplMetricsRecorderTest, IgnoresMetricsAfterDocumentInvalidation) {
    const std::chrono::nanoseconds duration(5000);
    auto timer = m_metricsRecorder->createTimer(m_document, "MyTimer");
    auto counter = m_metricsRecorder->createCounter(m_document, "MyCounter");

    m_metricsRecorder->invalidateDocument(m_document);

    timer->elapsed(duration);
    counter->increment();

    m_metricsRecorder->flush(); // strict mock will fail if anything is reported to the sink
}

TEST_F(AplMetricsRecorderTest, CleansUpOldDocuments) {
    const std::chrono::nanoseconds duration(5000);
    auto oldTimer1 = m_metricsRecorder->createTimer(m_document, "OldTimer1");
    auto oldTimer2 = m_metricsRecorder->createTimer(m_document, "OldTimer2");
    auto counter = m_metricsRecorder->createCounter(m_document, "OldCounter");
    counter->increment();

    oldTimer1->start();

    // Strict mock will fail if old metrics are reported
    EXPECT_CALL(*m_mockSink, reportTimer(IsEmpty(), "MyTimer", Eq(duration)))
        .Times(1);
    EXPECT_CALL(*m_mockSink, reportCounter(IsEmpty(), "OldCounter", Eq(1UL)))
        .Times(1); // old counter will be flushed because it has a value

    // Simulate rendering a new document while old metric handles are still active.
    auto newDocument = m_metricsRecorder->registerDocument();
    m_metricsRecorder->onRenderingStarted(newDocument);
    auto newTimer = m_metricsRecorder->createTimer(newDocument, "MyTimer");
    newTimer->elapsed(duration);
    m_metricsRecorder->onRenderingEnded(newDocument);

    m_metricsRecorder->flush();

    ASSERT_FALSE(oldTimer1->stop());
    ASSERT_FALSE(oldTimer2->start());
    ASSERT_FALSE(oldTimer2->stop());
    ASSERT_FALSE(counter->increment());
}

} // namespace test
} // namespace Telemetry
} // namespace APLClient
