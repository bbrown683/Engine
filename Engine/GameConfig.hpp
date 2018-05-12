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
	UserAudioSettings getUserAudioSettings();
	UserGraphicsSettings getUserGraphicsSettings();
	UserWindowSettings getUserWindowSettings();
	bool setDefaultSettings();
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