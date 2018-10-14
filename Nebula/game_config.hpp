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

enum class Quality {
	eLow,
	eMedium,
	eHigh,
	eUltra,
};

enum class TextureFiltering {
	eNone,
	e2x,
	e4x,
	e8x,
	e16x,
};

enum class Samples {
	eNone,
	e2x,
	e4x,
	e8x,
	e16x,
	e32x,
	e64x
};

enum class WindowMode {
	eWindowed,
	eFullscreen,
	eWindowedFullscreen,
};

class Config {
public:
	void setAudioMasterVolume(int volume);
	int getAudioMasterVolume();
	void setGraphicsVsync(bool vsync);
	bool getGraphicsVsync();
	void setGraphicsTripleBuffering(bool tripleBuffering);
	bool getGraphicsTripleBuffering();
	void setGraphicsTextureQuality(Quality textureQuality);
	Quality getGraphicsTextureQuality();
	void setGraphicsTextureFiltering(TextureFiltering textureFiltering);
	TextureFiltering getGraphicsTextureFiltering();
	void setWindowWidth(int width);
	int getWindowWidth();
	void setWindowHeight(int height);
	int getWindowHeight();
	void setWindowMode(WindowMode mode);
	WindowMode getWindowMode();
private:
	int volume;
	bool vsync;
	bool tripleBuffering;
	Quality textureQuality;
	TextureFiltering textureFiltering;
	int windowWidth, windowHeight;
	WindowMode windowMode;
};