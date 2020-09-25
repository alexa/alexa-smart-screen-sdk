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

#ifndef APL_CLIENT_LIBRARY_TELEMETRY_APL_METRICS_RECORDER
#define APL_CLIENT_LIBRARY_TELEMETRY_APL_METRICS_RECORDER

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "AplMetricsRecorderInterface.h"
#include "AplMetricsSinkInterface.h"

namespace APLClient {
namespace Telemetry {

class AplMetricsRecorder : public AplMetricsRecorderInterface,
                           public std::enable_shared_from_this<AplMetricsRecorder> {

private:
    AplMetricsRecorder(AplMetricsSinkInterfacePtr sink);

public:
    ~AplMetricsRecorder() override = default;

private:
    struct MetricRecord;
    struct DocumentRecord;
    class TimerHandle;
    class CounterHandle;

public:
    static AplMetricsRecorderInterfacePtr create(AplMetricsSinkInterfacePtr sink);

    DocumentId registerDocument() override;
    bool addMetadata(DocumentId document,
                     const std::string& key,
                     const std::string& value) override;
    void invalidateDocument(DocumentId documentId) override;
    DocumentId currentDisplayedDocument() const override;
    DocumentId latestDocument() const override;
    void flush() override;

    std::unique_ptr<AplTimerHandle> createTimer(DocumentId document,
                                                AplRenderingSegment segment,
                                                bool reportZeroFailures = false) override;
    std::unique_ptr<AplTimerHandle> createTimer(DocumentId document,
                                                const std::string &name,
                                                bool reportZeroFailures = false) override;
    std::unique_ptr<AplCounterHandle> createCounter(DocumentId document,
                                                    AplRenderingSegment segment,
                                                    bool reportZeroValues = true) override;
    std::unique_ptr<AplCounterHandle> createCounter(DocumentId document,
                                                    const std::string &name,
                                                    bool reportZeroValues = true) override;

    void onRenderingStarted(DocumentId document) override;
    void onRenderingEnded(DocumentId document) override;

private:
    bool updateTimer(DocumentId document, int id, std::function<bool(MetricRecord&)> updater);
    bool updateCounter(DocumentId document, int id, std::function<bool(MetricRecord&)> updater);
    void invalidateInactiveDocuments();
    bool isActive(DocumentId document);
    DocumentId resolveDocument(DocumentId document);
    void reportTimerIfNecessary(const DocumentRecord &documentRecord, MetricRecord &metricRecord);
    void reportCounterIfNecessary(const DocumentRecord &documentRecord, MetricRecord &metricRecord);

private:
    std::shared_ptr<AplMetricsSinkInterface> mSink;
    std::map<DocumentId, DocumentRecord> mDocuments;
    unsigned int mCurrentDocument;
    unsigned int mLatestDocument;
    unsigned int mNextDocument;
    std::mutex mDocumentMutex;
};

} // namespace Telemetry
} // namespace APLClient

#endif // APL_CLIENT_LIBRARY_TELEMETRY_APL_METRICS_RECORDER
