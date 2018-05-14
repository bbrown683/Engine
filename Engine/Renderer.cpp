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

#include "Renderer.hpp"
#include "DriverD3D12.hpp"
#include "DriverVk.hpp"

#include <iostream>

Renderer::Renderer(RendererDriver driver) : m_Driver(driver) {}

bool Renderer::createRendererForWindow(GLFWwindow* pWindow) {
    // This entire function will be logged eventually.

    if (!pWindow) {
        std::cerr << "FATAL: GLFW window is not a valid pointer!\n";
        return false;
    }
    if (m_Driver == RendererDriver::Direct3D12) {
        m_pDriver = std::make_unique<DriverD3D12>(pWindow);
        std::cout << "STATUS: Direct3D12 driver was selected...\n";
    }
    if (m_Driver == RendererDriver::Vulkan) {
        m_pDriver = std::make_unique<DriverVk>(pWindow);
        std::cout << "STATUS: Vulkan driver was selected...\n";
    }
    if (!m_pDriver->initialize()) {
        std::cerr << "FATAL: Failed to initialize render driver!\n";
        return false;
    }

    auto gpus = m_pDriver->getGpus();
    for (auto gpu : gpus)
        std::cout << gpu.name << std::endl;

    if (!m_pDriver->selectGpu(gpus.front().id)) {
        std::cerr << "CRITICAL: Failed to select GPU for operation!\n";
        return false;
    }
    return true;
}

RendererDriver Renderer::getRendererDriver() {
    return m_Driver;
}

bool Renderer::setRendererDriver(RendererDriver driver) {
    if (driver != m_Driver) {
        m_pDriver.reset();
    }
    return false;
}

bool Renderer::getVsync() {
    return m_Vsync;
}

void Renderer::setTextureFiltering(TextureFiltering textureFiltering) {
    m_TextureFiltering = textureFiltering;
}

TextureFiltering Renderer::getTextureFiltering() {
    return m_TextureFiltering;
}
