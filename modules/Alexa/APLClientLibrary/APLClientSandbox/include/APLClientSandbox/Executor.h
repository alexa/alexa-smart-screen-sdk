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

#ifndef APLCLIENTSANDBOX_INCLUDE_EXECUTOR_H_
#define APLCLIENTSANDBOX_INCLUDE_EXECUTOR_H_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <functional>

/**
 * A simple executor used to run callable types asynchronously.
 */
class Executor {
public:
    /**
     * Constructs an Executor.
     */
    Executor();

    /**
     * Destructs an Executor.
     */
    ~Executor();

    /**
     * Submits a callable type (function, lambda expression, bind expression, or another function object) to be executed
     * on an Executor thread.
     *
     * @param task A callable type representing a task.
     */
    template <typename Task>
    void submit(Task task);

    /// Clears the executor of outstanding tasks and refuses any additional tasks to be submitted.
    void shutdown();

private:
    // The main thread run loop
    void runner();

    /// The queue type to use for holding tasks.
    using Queue = std::deque<std::function<void()>>;

    /// The queue of tasks
    Queue m_queue;

    /// A mutex to protect access to the tasks in m_queue.
    std::mutex m_queueMutex;

    /// A flag for whether or not the queue is expecting more tasks.
    std::atomic_bool m_shutdown;

    /// The condition variable used to detect new job or timeout.
    std::condition_variable m_delayedCondition;

    /// The thread to execute tasks on. The thread must be declared last to be destructed first.
    std::thread m_thread;
};
template <typename Task>
void Executor::submit(Task task) {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_queue.push_back(std::move(task));
    lock.unlock();
    m_delayedCondition.notify_all();
}

#endif  // APLCLIENTSANDBOX_INCLUDE_EXECUTOR_H_
