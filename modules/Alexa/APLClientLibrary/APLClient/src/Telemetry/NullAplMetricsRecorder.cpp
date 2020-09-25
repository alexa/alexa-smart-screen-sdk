#include "APLClient/Telemetry/NullAplMetricsRecorder.h"

namespace APLClient {
namespace Telemetry {

AplMetricsRecorderInterface::DocumentId
NullAplMetricsRecorder::registerDocument() {
    return UNKNOWN_DOCUMENT;
}

bool
NullAplMetricsRecorder::addMetadata(DocumentId document,
                                    const std::string& key,
                                    const std::string& value) {
    return false;
}

void
NullAplMetricsRecorder::invalidateDocument(DocumentId documentId) {
    // empty
}

AplMetricsRecorderInterface::DocumentId
NullAplMetricsRecorder::currentDisplayedDocument() const {
    return UNKNOWN_DOCUMENT;
}

AplMetricsRecorderInterface::DocumentId
NullAplMetricsRecorder::latestDocument() const {
    return UNKNOWN_DOCUMENT;
}

void
NullAplMetricsRecorder::flush() {
    // empty
}

std::unique_ptr<AplTimerHandle>
NullAplMetricsRecorder::createTimer(DocumentId document,
                                    const std::string& name,
                                    bool reportZeroFailures) {
    return std::unique_ptr<AplTimerHandle>(new NullTimerHandle());
}

std::unique_ptr<AplTimerHandle>
NullAplMetricsRecorder::createTimer(DocumentId document,
                                    AplRenderingSegment segment,
                                    bool reportZeroFailures) {
    return std::unique_ptr<AplTimerHandle>(new NullTimerHandle());
}

std::unique_ptr<AplCounterHandle>
NullAplMetricsRecorder::createCounter(DocumentId document,
                                      const std::string& name,
                                      bool reportZero) {
    return std::unique_ptr<AplCounterHandle>(new NullCounterHandle());
}

std::unique_ptr<AplCounterHandle>
NullAplMetricsRecorder::createCounter(DocumentId document,
                                      AplRenderingSegment segment,
                                      bool reportZeroValues) {
    return std::unique_ptr<AplCounterHandle>(new NullCounterHandle());
}

void
NullAplMetricsRecorder::onRenderingStarted(DocumentId document) {
    // empty
}

void
NullAplMetricsRecorder::onRenderingEnded(DocumentId document) {
    // empty
}

} // namespace Telemetry
} // namespace APLClient