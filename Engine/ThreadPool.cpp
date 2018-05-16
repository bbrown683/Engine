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

#include "ThreadPool.hpp"

ThreadPool::ThreadPool(uint32_t threadCount) {
    for (uint32_t i = 0; i < threadCount; i++)
        m_Threads.emplace_back([this] {
        while(true) {
            std::function<void()> function;
            std::unique_lock<std::mutex> lock(this->m_Mutex);
            this->m_Condition.wait(lock, [this] { return this->m_Complete 
                || !this->m_Queue.empty(); });
            if (this->m_Complete && this->m_Queue.empty())
                return;
            function = std::move(this->m_Queue.front());
            this->m_Queue.pop();
            function();
        }
    });
}

void ThreadPool::enqueue(std::function<void()> function) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Queue.push(std::move(function));
    m_Condition.notify_one();
}

void ThreadPool::wait() {
    for (std::thread& thread : m_Threads) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Condition.wait(lock, [this] { return m_Queue.empty(); });
    }
}

ThreadPool::~ThreadPool() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Complete = true;
    m_Condition.notify_all();
    for (std::thread& thread : m_Threads)
        thread.join();
}