#pragma once

#include <libconfig.h>

#define CONFIG_VERSION 2

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
	bool vysnc;
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
	WindowMode windowMode;
};

class GameConfig {
public:
	GameConfig();
	~GameConfig();
	UserAudioSettings getUserAudioSettings();
	UserGraphicsSettings getUserGraphicsSettings();
	UserWindowSettings getUserWindowSettings();
	bool setAudioMasterVolume(float volume);
	bool setWindowX(int x);
	bool setWindowY(int y);
	bool setWindowWidth(int width);
	bool setWindowHeight(int height);
	bool setWindowMode(WindowMode mode);
	bool setGraphicsVsync(bool vsync);
	bool setGraphicsTextureQuality(Quality textureQuality);
	bool setGraphicsTextureFiltering(TextureFiltering textureFiltering);
private:
	config_t config;
	config_setting_t* audio, graphics, window, x, y, width, height, mode,
		vsync, textureQuality, textureFiltering, masterVolume; 
};