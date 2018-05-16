/*
MIT License

Copyright (c) 2018 Ben Brown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/// Simple threadpool implementation. 
class ThreadPool {
public:
    /// Spawns threadCount threads and has them execute in parallel, waiting until a task 
    /// is added to the queue a thread will pick up this task and execute it.
    ThreadPool(uint32_t threadCount);

    /// Ends the threadpool, returning all spawned threads.
    ~ThreadPool();

    /// Adds a funciton to the queue, where a thread will pick up the task and execute it.
    void enqueue(std::function<void()> function);

    /// Blocks execution on the calling thread until all spawned threads have returned.
    void wait();
private:
    std::vector<std::thread> m_Threads;
    std::queue<std::function<void()>> m_Queue;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    bool m_Complete;
};