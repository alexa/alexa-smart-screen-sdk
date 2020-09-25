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

#ifndef APL_CLIENT_LIBRARY_TELEMETRY_NULL_APL_METRICS_RECORDER
#define APL_CLIENT_LIBRARY_TELEMETRY_NULL_APL_METRICS_RECORDER

#include "AplMetricsRecorderInterface.h"

namespace APLClient {
namespace Telemetry {

class NullCounterHandle: public AplCounterHandle {
public:
    bool incrementBy(uint64_t value) override {
        return false;
    }
};

class NullTimerHandle : public AplTimerHandle {
    bool startedAt(const std::chrono::steady_clock::time_point& startTime) override {
        return false;
    };

    bool stoppedAt(const std::chrono::steady_clock::time_point& stopTime) override {
        return false;
    }

    bool elapsed(const std::chrono::nanoseconds& duration) override {
        return false;
    }

    bool fail() override {
        return false;
    }
};

class NullAplMetricsRecorder : public AplMetricsRecorderInterface {
public:
    NullAplMetricsRecorder() = default;
    ~NullAplMetricsRecorder() override = default;

    DocumentId registerDocument() override;
    bool addMetadata(DocumentId document,
                     const std::string &key,
                     const std::string &value) override;
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
                                                            bool reportZero = true) override;

    virtual std::unique_ptr<AplCounterHandle> createCounter(DocumentId document,
                                                            const std::string &name,
                                                            bool reportZero = true) override;

    void onRenderingStarted(DocumentId document) override;
    void onRenderingEnded(DocumentId document) override;
};


} // namespace Telemetry
} // namespace APLClient


#endif // ALEXASMARTSCREENSDK_NULLMETRICSRECORDER_H