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

#include <chrono>
#include <map>
#include <vector>

#include "APLClient/Telemetry/AplMetricsRecorder.h"
#include "APLClient/Telemetry/NullAplMetricsRecorder.h"

namespace APLClient {
namespace Telemetry {

static const std::map<AplRenderingSegment, std::string> sSegmentNames = {
    {AplRenderingSegment::kRenderDocument, "SmartScreenSDK.renderDocument"},
    {AplRenderingSegment::kContentCreation, "APL-Web.Content.create"},
    {AplRenderingSegment::kRootContextInflation, "APL.rootContext.inflate"},
    {AplRenderingSegment::kTextMeasure, "APL-Web.RootContext.measureCount"}
};

enum class MetricType { TIMER, COUNTER };

struct AplMetricsRecorder::MetricRecord {
    MetricType type;
    std::string name;
    bool reportZeroCounter;
    bool hasValue;
    int counterOrFailures;
    // Timer-specific fields
    std::chrono::steady_clock::time_point start;
    std::chrono::nanoseconds elapsed;
    bool started;
};

struct AplMetricsRecorder::DocumentRecord {
    std::map<std::string, std::string> metadata;
    std::vector<MetricRecord> metrics;
};

class AplMetricsRecorder::CounterHandle : public AplCounterHandle {
public:
    CounterHandle(std::shared_ptr<AplMetricsRecorder> recorder, DocumentId document, int id)
        : mRecorder{recorder}, mDocument{document}, mId{id} {};

    bool incrementBy(uint64_t value) override {
        if (auto recorder = mRecorder.lock()) {
            return recorder->updateCounter(mDocument, mId, [&](MetricRecord& record) {
                record.counterOrFailures += value;
                record.hasValue = true;
                return true;
            });
        }
        return false;
    }

private:
    std::weak_ptr<AplMetricsRecorder> mRecorder;
    DocumentId mDocument;
    int mId;
};

class AplMetricsRecorder::TimerHandle : public AplTimerHandle {
public:
    TimerHandle(std::shared_ptr<AplMetricsRecorder> recorder, DocumentId document, int id)
        : mRecorder{std::move(recorder)}, mDocument{document}, mId{id} {};

    bool startedAt(const std::chrono::steady_clock::time_point& startTime) override {
        if (auto recorder = mRecorder.lock()) {
            bool success = recorder->updateTimer(mDocument, mId, [&](MetricRecord& record) {
                if (record.started) {
                    // Avoid double starting
                    return false;
                }

                record.start = startTime;
                record.started = true;

                return true;
            });

            if (success && mStartCallback) {
                mStartCallback(*recorder);
            }

            return success;
        }

        return false;
    }

    bool stoppedAt(const std::chrono::steady_clock::time_point& stopTime) override {
        if (auto recorder = mRecorder.lock()) {
            bool success = recorder->updateTimer(mDocument, mId, [&](MetricRecord& record) {
                if (!record.started) {
                    return false;
                }

                auto duration = stopTime - record.start;
                record.elapsed += duration;
                record.started = false;
                record.hasValue = true;
                return true;
            });

            if (success && mStopCallback) {
                mStopCallback(*recorder);
            }

            return success;
        }
        return false;
    }

    bool elapsed(const std::chrono::nanoseconds& duration) override {
        if (auto recorder = mRecorder.lock()) {
            bool success = recorder->updateTimer(mDocument, mId, [&](MetricRecord& record) {
                record.elapsed += duration;
                record.hasValue = true;
                return true;
            });

            if (success && mStopCallback) {
                mStopCallback(*recorder);
            }

            return success;
        }
        return false;
    }

    bool fail() override {
        if (auto recorder = mRecorder.lock()) {
            bool success = recorder->updateTimer(mDocument, mId, [&](MetricRecord& record) {
                record.counterOrFailures += 1;
                record.started = false;
                return true;
            });

            if (success && mStopCallback) {
                mStopCallback(*recorder);
            }

            return success;
        }
        return false;
    }

    void setStartCallback(std::function<void(AplMetricsRecorder&)> callback) {
        mStartCallback = callback;
    }

    void setStopCallback(std::function<void(AplMetricsRecorder&)> callback) {
        mStopCallback = callback;
    }

private:
    std::weak_ptr<AplMetricsRecorder> mRecorder;
    DocumentId mDocument;
    int mId;
    std::function<void(AplMetricsRecorder&)> mStartCallback;
    std::function<void(AplMetricsRecorder&)> mStopCallback;
};

AplMetricsRecorderInterfacePtr
AplMetricsRecorder::create(AplMetricsSinkInterfacePtr sink) {
    return AplMetricsRecorderInterfacePtr(new AplMetricsRecorder(std::move(sink)));
}

AplMetricsRecorder::AplMetricsRecorder(AplMetricsSinkInterfacePtr sink)
    : mSink{sink},
      mDocuments{},
      mCurrentDocument{UNKNOWN_DOCUMENT},
      mLatestDocument{UNKNOWN_DOCUMENT},
      mNextDocument{LATEST_DOCUMENT + 1} {
    // empty
}

AplMetricsRecorderInterface::DocumentId
AplMetricsRecorder::registerDocument() {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);

    DocumentId id = mNextDocument++;
    if (id == UNKNOWN_DOCUMENT) {
        // Wrapped around, start again from the first valid ID
        mNextDocument = LATEST_DOCUMENT + 1;
        id = mNextDocument++;
    }

    DocumentRecord record;
    mDocuments.emplace(id, record);
    return id;
}

bool
AplMetricsRecorder::addMetadata(DocumentId document,
                                const std::string& key, const std::string& value) {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);

    document = resolveDocument(document);
    if (mDocuments.count(document) == 0) {
        return false;
    }

    DocumentRecord &record = mDocuments.at(document);
    record.metadata[key] = value;
    return true;
}

void
AplMetricsRecorder::invalidateDocument(DocumentId document) {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);

    document = resolveDocument(document);
    if (mCurrentDocument == document) {
        mCurrentDocument = UNKNOWN_DOCUMENT;
    }

    if (mLatestDocument == document) {
        mLatestDocument = UNKNOWN_DOCUMENT;
    }

    mDocuments.erase(document);
}

AplMetricsRecorderInterface::DocumentId
AplMetricsRecorder::currentDisplayedDocument() const {
    return mCurrentDocument;
}

AplMetricsRecorderInterface::DocumentId
AplMetricsRecorder::latestDocument() const {
    return mLatestDocument;
}

void
AplMetricsRecorder::onRenderingStarted(DocumentId document) {
    mLatestDocument = document;
}

void
AplMetricsRecorder::onRenderingEnded(DocumentId document) {
    mCurrentDocument = document;

    flush();
    invalidateInactiveDocuments();
}

void
AplMetricsRecorder::flush() {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);

    for (auto &documentEntry : mDocuments) {
        auto &documentRecord = documentEntry.second;
        for (MetricRecord& metricRecord : documentRecord.metrics) {
            if (metricRecord.type == MetricType::TIMER) {
                reportTimerIfNecessary(documentRecord, metricRecord);
            }
            else if (metricRecord.type == MetricType::COUNTER) {
                reportCounterIfNecessary(documentRecord, metricRecord);
            }
        }
    }
}

void
AplMetricsRecorder::reportTimerIfNecessary(const DocumentRecord &documentRecord, MetricRecord& metricRecord) {
    if (metricRecord.started) {
        return;  // Timer in progress, skip it
    }

    bool shouldReportTimer = metricRecord.hasValue;
    bool shouldReportFailure = metricRecord.counterOrFailures > 0 ||
        (shouldReportTimer && metricRecord.reportZeroCounter);
    if (shouldReportTimer) {
        mSink->reportTimer(documentRecord.metadata,
                           metricRecord.name,
                           metricRecord.elapsed);

        metricRecord.elapsed = std::chrono::nanoseconds::zero();
        metricRecord.hasValue = false;
    }

    if (shouldReportFailure) {
        mSink->reportCounter(documentRecord.metadata,
                             metricRecord.name + ".fail",
                             metricRecord.counterOrFailures);
        metricRecord.counterOrFailures = 0;
    }
}

void
AplMetricsRecorder::reportCounterIfNecessary(const DocumentRecord &documentRecord, MetricRecord& metricRecord) {
    if (!metricRecord.hasValue) {
        return;
    }

    mSink->reportCounter(documentRecord.metadata, metricRecord.name,
                         metricRecord.counterOrFailures);
    metricRecord.counterOrFailures = 0;
    metricRecord.hasValue = false;
}

void
AplMetricsRecorder::invalidateInactiveDocuments() {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);

    for (auto it = mDocuments.begin(); it != mDocuments.end(); ) {
        DocumentId document = it->first;
        if (isActive(document)) {
            ++it;
        } else {
            it = mDocuments.erase(it);
        }
    }
}

bool
AplMetricsRecorder::isActive(DocumentId document) {
    if (document == mCurrentDocument) {
        return true;
    }

    if (mNextDocument > mLatestDocument) {
        // expected case, document IDs increase monotonically. Make sure this document
        // is between latest and the next unused ID
        return document >= mLatestDocument && document < mNextDocument;
    } else {
        // document IDs wrapped around, so the valid id range is split
        return document >= mLatestDocument || document < mNextDocument;
    }
}

std::unique_ptr<AplTimerHandle>
AplMetricsRecorder::createTimer(AplMetricsRecorderInterface::DocumentId document,
                                AplRenderingSegment segment,
                                bool reportZeroFailures) {
    if (sSegmentNames.count(segment) == 0) {
        return std::unique_ptr<AplTimerHandle>(new NullTimerHandle());
    }

    auto timerHandle = createTimer(document, sSegmentNames.at(segment), reportZeroFailures);
    if (timerHandle && segment == AplRenderingSegment::kRenderDocument) {
        TimerHandle *handle = (TimerHandle*) timerHandle.get();
        handle->setStartCallback([document](AplMetricsRecorder &recorder) {
          recorder.onRenderingStarted(document);
        });
        handle->setStopCallback([document](AplMetricsRecorder& recorder) {
          recorder.onRenderingEnded(document);
        });
    }
    return timerHandle;
}

std::unique_ptr<AplTimerHandle>
AplMetricsRecorder::createTimer(DocumentId document,
                                const std::string &name,
                                bool reportZeroFailures) {
    // Replace 'special' document id, if used
    document = resolveDocument(document);
    if (mDocuments.count(document) == 0) {
        return std::unique_ptr<AplTimerHandle>(new NullTimerHandle());
    }

    const std::lock_guard<std::mutex> lock(mDocumentMutex);
    MetricRecord record;
    record.type = MetricType::TIMER;
    record.name = name;
    record.elapsed = std::chrono::nanoseconds::zero();
    record.counterOrFailures = 0;
    record.started = false;
    record.hasValue = false;
    record.reportZeroCounter = reportZeroFailures;
    auto& records = mDocuments.at(document).metrics;
    int id = records.size();
    records.emplace_back(record);

    auto timerHandle = new TimerHandle(shared_from_this(), document, id);
    return std::unique_ptr<TimerHandle>(timerHandle);
}

std::unique_ptr<AplCounterHandle>
AplMetricsRecorder::createCounter(DocumentId document,
                                  AplRenderingSegment segment,
                                  bool reportZeroValues) {
    if (sSegmentNames.count(segment) == 0) {
        return std::unique_ptr<AplCounterHandle>(new NullCounterHandle());
    }

    return createCounter(document, sSegmentNames.at(segment), reportZeroValues);
}

std::unique_ptr<AplCounterHandle>
AplMetricsRecorder::createCounter(DocumentId document,
                                  const std::string &name,
                                  bool reportZeroValues) {
    // Replace 'special' document id, if used
    document = resolveDocument(document);

    if (mDocuments.count(document) == 0) {
        return std::unique_ptr<AplCounterHandle>(new NullCounterHandle());
    }

    const std::lock_guard<std::mutex> lock(mDocumentMutex);
    MetricRecord record;
    record.type = MetricType::COUNTER;
    record.name = name;
    record.elapsed = std::chrono::nanoseconds::zero();
    record.counterOrFailures = 0;
    record.started = false;
    record.hasValue = reportZeroValues;
    record.reportZeroCounter = reportZeroValues;
    auto& records = mDocuments.at(document).metrics;
    int id = records.size();
    records.emplace_back(record);

    return std::unique_ptr<CounterHandle>(new CounterHandle(shared_from_this(), document, id));
}

bool
AplMetricsRecorder::updateTimer(DocumentId document,
                                int id,
                                std::function<bool(MetricRecord&)> updater) {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);
    if (mDocuments.count(document) == 0) {
        return false;
    }

    auto& records = mDocuments.at(document).metrics;
    if (id < 0 || id >= records.size()) {
        return false;
    }

    MetricRecord& record = records.at(id);
    if (record.type != MetricType::TIMER) {
        return false;
    }

    return updater(record);
}

bool
AplMetricsRecorder::updateCounter(DocumentId document,
                                  int id,
                                  std::function<bool(MetricRecord&)> updater) {
    const std::lock_guard<std::mutex> lock(mDocumentMutex);
    if (mDocuments.count(document) == 0) {
        return false;
    }
    auto& records = mDocuments.at(document).metrics;
    if (id < 0 || id >= records.size()) {
        return false;
    }

    MetricRecord& record = records.at(id);
    if (record.type != MetricType::COUNTER) {
        return false;
    }

    return updater(record);
}

AplMetricsRecorderInterface::DocumentId
AplMetricsRecorder::resolveDocument(DocumentId document) {
    if (document == CURRENT_DOCUMENT) {
        if (mCurrentDocument != UNKNOWN_DOCUMENT) {
            return mCurrentDocument;
        } else {
            return mLatestDocument;
        }
    } else if (document == LATEST_DOCUMENT) {
        return mLatestDocument;
    } else {
        return document;
    }
}

} // namespace Telemetry
} // namespace APLClient
