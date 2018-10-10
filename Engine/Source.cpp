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

#include <vector>

#include "EngineRenderer.hpp"
#include "HelperDX.hpp"

#pragma warning(push)
#pragma warning(disable : 4018)
#pragma warning(disable : 4996)
#define LOGURU_IMPLEMENTATION 1
#include "thirdparty/loguru/loguru.hpp"
#pragma warning(pop)
#define STB_IMAGE_IMPLEMENTATION 1
#include "thirdparty/stb/stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION 1
#include "thirdparty/stb/stb_truetype.h"
#define TINYOBJLOADER_IMPLEMENTATION 1
#include "thirdparty/tinyobjloader/tiny_obj_loader.h"

int main(int argc, char** argv) {
	loguru::init(argc, argv);

#ifndef _DEBUG
	loguru::add_file("runtime.log", loguru::Truncate, loguru::Verbosity_MAX);
#endif

	std::vector<const char*> args;
	args.insert(args.begin(), argv, argv + argc);
	RendererDriver driver = RendererDriver::eAutodetect;
	for (const char* arg : args) {
		if (std::strcmp(arg, "--dx") == 0)
			driver = RendererDriver::eDirectX;
		if (std::strcmp(arg, "--vk") == 0)
			driver = RendererDriver::eVulkan;
	}

	EngineRenderer engine(driver);
	if (!engine.initialize())
		return false;

	return engine.executeEventLoop();
}