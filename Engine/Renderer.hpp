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

#include <memory>

#include "Driver.hpp"
#include "Scene.hpp"

enum class RendererDriver {
    /// Renderer will select the driver to use.
    eAutodetect,
    /// Renderer will select Direct3D12 driver. 
    eDirect3D12,
    /// Renderer will select select the Vulkan driver.
    eVulkan,
};

enum class WindowMode {
    eBorderless,
    eFullscreen,
    eWindowed,
};

enum class TextureFiltering {
    eNone,
    eBilinear,
    eTrilinear,
    eAniso2x,
    eAniso4x,
    eAniso8x,
    eAniso16x,
};

class Renderer {
public:
    /// Constructs a renderer with the specified rendering driver.
    /// After instantiation you will need to call createRendererForWindow
    /// to complete object creation.
    explicit Renderer(RendererDriver driver);

    /// This function initializes the renderer class for the given GLFW window.
    /// This must be the first function called after creating the object. This 
    /// call will return the status of whether the renderer was successfully created.
    bool createRendererForWindow(const SDL_Window* pWindow);

    ///
    ///
    ///
    bool drawScene(Scene* scene);

    /// This function will return the renderer driver currently being used
    /// by the renderer. This will not return Autodetect, which will select either
    /// Direct3D12 or Vulkan, which are the only two possible return options for
    /// this function.
    RendererDriver getRendererDriver();

    /// Sets the specified renderer driver for use. If the renderer driver differs from the 
    /// currently active driver, it will cleanup the previous driver and reinitialize with
    /// new driver.
    bool setRendererDriver(RendererDriver driver);

    /// Returns the current state of Vsync for the renderer. 
    bool getVsync();

    /// Sets the current state of Vsync for the renderer. This can be in the form of either
    /// double or triple buffering depending on the backend driver implementation.
    void setVsync(bool state);

    /// Returns the currently applied texture filtering for the renderer.
    TextureFiltering getTextureFiltering();

    /// 
    void setTextureFiltering(TextureFiltering textureFiltering);
private:
    std::unique_ptr<Driver> m_pDriver;
    RendererDriver m_Driver;
    bool m_Vsync;
    TextureFiltering m_TextureFiltering;
};
