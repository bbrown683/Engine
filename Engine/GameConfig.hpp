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

#include <libconfig.h>

struct UserAudioSettings {
	int masterVolume;
};

enum class Quality {
	Low,
	Medium,
	High,
	Ultra,
};

enum class TextureFiltering {
	None,
	Anisotropic2x,
	Anisotropic4x,
	Anisotropic8x,
	Anisotropic16x
};

struct UserGraphicsSettings {
	bool vsync;
	Quality textureQuality;
	TextureFiltering textureFiltering;
};

enum class WindowMode {
	Windowed,
	Fullscreen,
	WindowedFullscreen,
};

struct UserWindowSettings {
	int x, y;
	int width, height;
	WindowMode mode;
};

class GameConfig {
public:
	GameConfig();
	~GameConfig();
	bool useDefaultSettings();
	UserAudioSettings getUserAudioSettings();
	UserGraphicsSettings getUserGraphicsSettings();
	UserWindowSettings getUserWindowSettings();
	bool setAudioMasterVolume(int volume);
	bool setGraphicsVsync(bool vsync);
	bool setGraphicsTextureQuality(Quality textureQuality);
	bool setGraphicsTextureFiltering(TextureFiltering textureFiltering);
	bool setWindowX(int x);
	bool setWindowY(int y);
	bool setWindowWidth(int width);
	bool setWindowHeight(int height);
	bool setWindowMode(WindowMode mode);
private:
	bool writeConfig();

	const char* configFile = "settings.cfg";
	config_t config;
	config_setting_t* root;
	config_setting_t* audio;
	config_setting_t* graphics;
	config_setting_t* window;
	config_setting_t* masterVolume;
	config_setting_t* vsync; 
	config_setting_t* textureQuality; 
	config_setting_t* textureFiltering; 
	config_setting_t* x;
	config_setting_t* y;
	config_setting_t* width;
	config_setting_t* height;
	config_setting_t* mode;
	UserAudioSettings audioSettings;
	UserGraphicsSettings graphicsSettings;
	UserWindowSettings windowSettings;
};