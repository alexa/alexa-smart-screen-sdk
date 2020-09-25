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

#ifndef APL_CLIENT_LIBRARY_METRICS_RECORDER_INTERFACE
#define APL_CLIENT_LIBRARY_METRICS_RECORDER_INTERFACE

#include <stdint.h>

#include <chrono>
#include <memory>
#include <string>

namespace APLClient {
namespace Telemetry {

/**
 * A handle to a registered timer. Each timer is associated with an APL document. If the APL document gets
 * invalidated while a handle is still active, the handle essentially becomes a no-op.
 */
class AplTimerHandle {
public:
    AplTimerHandle() = default;
    virtual ~AplTimerHandle() = default;

    /**
     * Starts the timer at the current time.
     *
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool start() {
        auto startTime = std::chrono::steady_clock::now();
        return startedAt(startTime);
    }

    /**
     * Stops the timer at the current time.
     *
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool stop() {
        auto stopTime = std::chrono::steady_clock::now();
        return stoppedAt(stopTime);
    }

    /**
     * Starts the timer at the specified time.
     *
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool startedAt(const std::chrono::steady_clock::time_point& startTime) = 0;

    /**
     * Stops the timer at the specified time.
     *
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool stoppedAt(const std::chrono::steady_clock::time_point& stopTime) = 0;

    /**
     * Records the specified duration for this timer without the need to explicitly start/stop.
     *
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool elapsed(const std::chrono::nanoseconds& duration) = 0;

    /**
     * Records a failure for the specified timer. The timer will be deactivated without stopping, i.e. no duration
     * will be recorded for it. A failure counter will be emitted instead.
     *
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool fail() = 0;
};

/**
 * A handle to a registered counter. Each counter is associated with an APL document. If the APL document gets
 * invalidated while a handle is still active, the handle essentially becomes a no-op.
 */
class AplCounterHandle {
public:
    AplCounterHandle() = default;
    virtual ~AplCounterHandle() = default;

    /**
     * Increments the counter by 1.
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool increment() { return incrementBy(1u); }

    /**
     * Increments the counter by the specified amount.
     *
     * @param value the amount of increment by
     * @return @c true if successful, @c false otherwise (e.g. handle refers to an invalidated document)
     */
    virtual bool incrementBy(uint64_t value) = 0;
};

enum class AplRenderingSegment {
    /** Corresponds to the overall rendering of an APL document */
    kRenderDocument,
    /** Corresponds to the creation of the APL @c Content object, including downloading imports */
    kContentCreation,
    /** Corresponds to inflating the APL @c RootContext object */
    kRootContextInflation,
    /** Corresponds to performing a text measuring requested by APL during layout */
    kTextMeasure
};

/**
 * The contract for metrics recording from APL code. Implementations are allowed to buffer recorded metrics
 * in order to limit possibly expensive reporting.
 */
class AplMetricsRecorderInterface {
public:
    /** Opaque type for document IDs. Clients should not rely on DocumentId being a specific type. */
    using DocumentId = unsigned int;

    /** Special document ID that identifies an unknown (e.g. previously invalidated) document. */
    static const DocumentId UNKNOWN_DOCUMENT;

    /** Special document ID that is internally mapped to the document ID of the currently displayed document, if any. */
    static const DocumentId CURRENT_DOCUMENT;

    /** Special document ID that is internally mapped to the document ID of the document ID from the last
     * RenderDocument directive. */
    static const DocumentId LATEST_DOCUMENT;

    virtual ~AplMetricsRecorderInterface() = default;

    /**
     * Registers a new document, typically in response to starting to process a RenderDocument directive.
     */
    virtual DocumentId registerDocument() = 0;

    /**
     * Adds metadata (key-value pair) to a previously registered document.
     *
     * @param document The document to modify, or one of the special document IDs (@c CURRENT_DOCUMENT or @c
     * LATEST_DOCUMENT).
     * @param key The key in the key-value pair
     * @param value The value in the key-value pair
     */
    virtual bool addMetadata(DocumentId document,
                             const std::string &key,
                             const std::string &value) = 0;

    /**
     * Invalidates the specified document, if active.
     *
     * @param documentId The document to invalidate, or one of the special document IDs (@c CURRENT_DOCUMENT or @c
     * LATEST_DOCUMENT).
     */
    virtual void invalidateDocument(DocumentId documentId) = 0;

    /**
     * Returns the currently displayed document.
     *
     * @return The ID of the currently displayed document, or @c UNKNOWN_DOCUMENT if no such document exists.
     */
    virtual DocumentId currentDisplayedDocument() const = 0;

    /**
     * Returns the latest registered document.
     *
     * @return The ID of the latest registered document, or @c UNKNOWN_DOCUMENT if no such document exists.
     */
    virtual DocumentId latestDocument() const = 0;

    /**
     * Causes all pending telemetry to be emitted to the sink, if any buffered telemetry is present.
     */
    virtual void flush() = 0;

    /**
     * Creates a timer for the specified rendering segment.
     *
     * @param document The document associated with this timer (special document IDs are allowed)
     * @param segment The rendering segment to time
     * @param reportZeroFailures Whether a failure counter should be emitted even if no failure is registered for
     * this segment
     * @return A pointer to the newly created timer handle, or @c nullptr if an error is encountered.
     */
    virtual std::unique_ptr<AplTimerHandle> createTimer(DocumentId document,
                                                        AplRenderingSegment segment,
                                                        bool reportZeroFailures = false) = 0;

    /**
     * Creates a timer for with the specified name.
     *
     * @param document The document associated with this timer (special document IDs are allowed)
     * @param segment The name of the timer
     * @param reportZeroFailures Whether a failure counter should be emitted even if no failure is registered for
     * this segment
     * @return A pointer to the newly created timer handle, or @c nullptr if an error is encountered.
     */
    virtual std::unique_ptr<AplTimerHandle> createTimer(DocumentId document,
                                                        const std::string &name,
                                                        bool reportZeroFailures = false) = 0;

    /**
     * Creates a counter for the specified rendering segment.
     *
     * @param document The document associated with this counter (special document IDs are allowed)
     * @param segment The rendering segment to count
     * @param reportZero Whether the counter should be emitted even if it is never explicitly incremented.
     * @return A pointer to the newly created counter handle, or @c nullptr if an error is encountered.
     */
    virtual std::unique_ptr<AplCounterHandle> createCounter(DocumentId document,
                                                            AplRenderingSegment segment,
                                                            bool reportZero = true) = 0;

    /**
     * Creates a counter with the specified name.
     *
     * @param document The document associated with this counter (special document IDs are allowed)
     * @param name The name of the counter
     * @param reportZero Whether the counter should be emitted even if it is never explicitly incremented.
     * @return A pointer to the newly created counter handle, or @c nullptr if an error is encountered.
     */
    virtual std::unique_ptr<AplCounterHandle> createCounter(DocumentId document,
                                                            const std::string &name,
                                                            bool reportZero = true) = 0;

    /**
     * Invoked when rendering starts for the specified document. Used to maintain internal state.
     *
     * @param document The document is being rendered. Special document IDs should not be used.
     */
    virtual void onRenderingStarted(DocumentId document) = 0;

    /**
     * Invoked when rendering stops for the specified document. Used to maintain internal state.
     *
     * @param document The document that was rendered. Special document IDs should not be used.
     */
    virtual void onRenderingEnded(DocumentId document) = 0;
};

using AplMetricsRecorderInterfacePtr = std::shared_ptr<AplMetricsRecorderInterface>;

}; // namespace Telemetry
}; // namespace APLClient

#endif // APL_CLIENT_LIBRARY_METRICS_RECORDER_INTERFACE
