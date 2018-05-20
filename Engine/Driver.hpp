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

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "ThreadPool.hpp"

struct Gpu {
    uint32_t id;
    char name[256];
    uint32_t memory;
    bool software;
};

enum class RenderableType {
    Mesh,
    Font,
};

struct SDL_Window;
class Renderable;
class Driver {
public:
    explicit Driver(const SDL_Window* pWindow);
    virtual ~Driver() {}

    /// Initializes all of the driver specific state in order for it to properly function.
    /// This should be the first function called after instantiating the driver object.
    /// It will return true when the driver was successfully initialized, and false if it
    /// encountered an error. Any error on this function should be treated as fatal and 
    /// should cause the application to shutdown.
    virtual bool initialize() = 0;

    /// Selects the input GPU for operation. This function will return true if
    /// the Gpu was successfully selected. If this returns false, it should be treated 
    /// as a fatal error if it is the only available Gpu on the system. You can try another
    /// Gpu and see if it succeeds otherwise.
    virtual bool selectGpu(uint32_t id) = 0;

    /// Submits all of the gathered command buffers/lists to the GPU for execution.
    /// This call will block until all GPU execution has completed. Will present
    /// all information executed in the queue to the swapchain for presentation.
    virtual bool presentFrame() = 0;

    /// Returns a list of all GPUs along with information about each one of them.
    /// id - The identifier of this GPU.
    /// name - The name of this GPU.
    /// memory - The maximum amount of video memory for this GPU.
    /// software - A boolean of whether this GPU is a software rasterizer and not a physical graphics card. 
    const std::vector<Gpu>& getGpus();
protected:
    const SDL_Window* getWindow();
    void addGpu(Gpu gpu);
    uint32_t getThreadCount();
    ThreadPool* getThreadPool();

    /// Creates a renderable object which at runtime is dynamically filled
    /// with rendering commands. After these commands are filled, it is
    /// pushed into a buffer where it is ready to be submitted. With the
    /// once flag set, this renderable will expire and will need to be 
    /// recreated.
    virtual std::unique_ptr<Renderable> createRenderable() = 0;
private:
    const SDL_Window* m_pWindow;
    std::vector<Gpu> m_Gpus;
    uint32_t m_ThreadCount;
    std::unique_ptr<ThreadPool> m_ThreadPool;
};