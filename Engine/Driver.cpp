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

#include "Driver.hpp"

#include <iostream>

Driver::Driver(const SDL_Window* pWindow) : m_pWindow(pWindow) {
    m_ThreadCount = std::thread::hardware_concurrency();
    m_ThreadPool = std::make_unique<ThreadPool>(m_ThreadCount);
}

MaxAnisotropy Driver::getMaxAnisotropy() {
    return m_MaxAnisotropy;
}

const std::vector<Gpu>& Driver::getGpus() {
    return m_Gpus;
}

const SDL_Window* Driver::getWindow() {
    return m_pWindow;
}

void Driver::setMaxAnisotropy(MaxAnisotropy anisotropyLevel) {
    m_MaxAnisotropy = anisotropyLevel;
}

void Driver::addGpu(Gpu gpu) {
    m_Gpus.push_back(gpu);
}

uint32_t Driver::getThreadCount() {
    return m_ThreadCount;
}

ThreadPool* Driver::getThreadPool() {
    return m_ThreadPool.get();
}
