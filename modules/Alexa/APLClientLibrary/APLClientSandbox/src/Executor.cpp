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

#include "APLClientSandbox/Executor.h"

Executor::~Executor() {
    shutdown();
}

Executor::Executor() : m_shutdown{false} {
    m_thread = std::thread(&Executor::runner, this);
}

void Executor::shutdown() {
    std::unique_lock<std::mutex> lock{m_queueMutex};
    m_queue.clear();
    m_shutdown = true;
    lock.unlock();
    m_delayedCondition.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Executor::runner() {
    std::unique_lock<std::mutex> lock(m_queueMutex);

    do {
        // Wait until we have data or a quit signal
        m_delayedCondition.wait(lock, [this] { return (!m_queue.empty() || m_shutdown); });

        // after wait, we own the lock
        if (!m_shutdown && !m_queue.empty()) {
            auto op = std::move(m_queue.front());
            m_queue.pop_front();

            // unlock now that we're done messing with the queue
            lock.unlock();

            op();

            lock.lock();
        }
    } while (!m_shutdown);
}