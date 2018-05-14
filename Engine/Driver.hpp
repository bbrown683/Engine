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
#include <vector>

struct Gpu {
    uint8_t id;
    char name[256];
    uint32_t memory;
    bool software;
};

struct GLFWwindow;

class Driver {
public:
    explicit Driver(GLFWwindow* pWindow) : m_pWindow(pWindow) {}
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
    virtual bool selectGpu(uint8_t id) = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    /// Returns a list of all GPUs along with information about each one of them.
    /// id - The identifier of this GPU.
    /// name - The name of this GPU.
    /// memory - The maximum amount of video memory for this GPU.
    /// software - A boolean of whether this GPU is a software rasterizer and not a physical graphics card. 
    std::vector<Gpu> getGpus() { return m_Gpus; };
protected:
    GLFWwindow * getWindow() const { return m_pWindow; };
    void addGpu(Gpu gpu) { m_Gpus.push_back(gpu); };
private:
    GLFWwindow * m_pWindow;
    std::vector<Gpu> m_Gpus;
};