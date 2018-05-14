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

#include "AbstractDriver.hpp"
#include "DriverD3D.hpp"
#include "DriverVk.hpp"

enum class RendererDriver {
	eAutodetect,
	eDirect3D,
	eVulkan,
};

enum class TextureFiltering {
	Default,
	Bilinear,
	Trilinear,
	Aniso2x,
	Aniso4x,
	Aniso8x,
	Aniso16x,
};

class Renderer {
public:
	explicit Renderer(RendererDriver driver);

	/// This function initializes the renderer class for the given GLFW window.
	/// This must be the first function called after creating the object. This 
	/// call will return the status of whether the renderer was successfully created.
	bool createRendererForWindow(GLFWwindow* pWindow);
	
	/// This function will return the renderer driver currently being used
	/// by the renderer. This will not return eAutodetect, which will select either
	/// eDirect3D12 or eVulkan, which are the only two possible return options from
	/// this function.
	RendererDriver getRendererDriver();
	bool setRendererDriver(RendererDriver driver);
	void setVsync(bool state);
	bool getVsync();
	void setTextureFiltering(TextureFiltering textureFiltering);
	TextureFiltering getTextureFiltering();
private:
	std::unique_ptr<AbstractDriver> pDriver;
	RendererDriver m_Driver;
	bool vsync;
	TextureFiltering textureFiltering;
};
