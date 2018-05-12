#include "GameConfig.hpp"

GameConfig::GameConfig() {
	config_init(&config);
	root = config_root_setting(&config);
	audio = config_setting_add(root, "audio", CONFIG_TYPE_GROUP);
	graphics = config_setting_add(root, "graphics", CONFIG_TYPE_GROUP);
	window = config_setting_add(root, "window", CONFIG_TYPE_GROUP);
	vsync = config_setting_add(graphics, "vsync", CONFIG_TYPE_BOOL);
	textureQuality = config_setting_add(graphics, "textureQuality", CONFIG_TYPE_INT);
	textureFiltering = config_setting_add(graphics, "textureFiltering", CONFIG_TYPE_INT);
	x = config_setting_add(window, "x", CONFIG_TYPE_INT);
	y = config_setting_add(window, "y", CONFIG_TYPE_INT);
	width = config_setting_add(window, "width", CONFIG_TYPE_INT);
	height = config_setting_add(window, "height", CONFIG_TYPE_INT);
	mode = config_setting_add(window, "mode", CONFIG_TYPE_INT);
	masterVolume = config_setting_add(audio, "masterVolume", CONFIG_TYPE_FLOAT);
}

GameConfig::~GameConfig() {
	config_destroy(&config);
}

bool GameConfig::useDefaultSettings() {
	audioSettings.masterVolume = 100;
	config_setting_set_float(masterVolume, audioSettings.masterVolume);
	graphicsSettings.vsync = true;
	config_setting_set_bool(vsync, graphicsSettings.vsync);
	graphicsSettings.textureQuality = Quality::Low;
	config_setting_set_int(textureQuality, static_cast<int>(graphicsSettings.textureQuality));
	graphicsSettings.textureFiltering = TextureFiltering::None;
	config_setting_set_int(textureFiltering, static_cast<int>(graphicsSettings.textureFiltering));
	windowSettings.x = 0;
	config_setting_set_int(x, windowSettings.x);
	windowSettings.y = 0;
	config_setting_set_int(y, windowSettings.y);
	windowSettings.width = 1024;
	config_setting_set_int(width, windowSettings.width);
	windowSettings.height = 768;
	config_setting_set_int(height, windowSettings.height);
	windowSettings.mode = WindowMode::Windowed;
	config_setting_set_int(mode, static_cast<int>(windowSettings.mode));
	return writeConfig();
}

UserAudioSettings GameConfig::getUserAudioSettings() {
	return audioSettings;
}

UserGraphicsSettings GameConfig::getUserGraphicsSettings() {
	return graphicsSettings;
}

UserWindowSettings GameConfig::getUserWindowSettings() {
	return windowSettings;
}

bool GameConfig::setAudioMasterVolume(int masterVolume) {
	audioSettings.masterVolume = masterVolume;
	config_setting_set_int(this->masterVolume, audioSettings.masterVolume);
	return writeConfig();
}

bool GameConfig::setGraphicsVsync(bool vsync) {
	graphicsSettings.vsync = vsync;
	config_setting_set_bool(this->vsync, graphicsSettings.vsync);
	return writeConfig();
}

bool GameConfig::setGraphicsTextureQuality(Quality textureQuality) {
	graphicsSettings.textureQuality = textureQuality;
	config_setting_set_int(this->textureQuality, static_cast<int>(graphicsSettings.textureQuality));
	return writeConfig();
}

bool GameConfig::setGraphicsTextureFiltering(TextureFiltering textureFiltering) {
	graphicsSettings.textureFiltering = textureFiltering;
	config_setting_set_int(this->textureFiltering, static_cast<int>(graphicsSettings.textureFiltering));
	return writeConfig();
}

bool GameConfig::setWindowX(int x) {
	windowSettings.x = x;
	config_setting_set_int(this->x, windowSettings.x);
	return writeConfig();
}

bool GameConfig::setWindowY(int y) {
	windowSettings.y = y;
	config_setting_set_int(this->y, windowSettings.y);
	return writeConfig();
}

bool GameConfig::setWindowWidth(int width) {
	windowSettings.width = width;
	config_setting_set_int(this->width, windowSettings.width);
	return writeConfig();
}

bool GameConfig::setWindowHeight(int height) {
	windowSettings.height = height;
	config_setting_set_int(this->height, windowSettings.height);
	return writeConfig();
}

bool GameConfig::setWindowMode(WindowMode mode) {
	windowSettings.mode = mode;
	config_setting_set_int(this->mode, static_cast<int>(windowSettings.mode));
	return writeConfig();
}

bool GameConfig::writeConfig() {
	if (config_write_file(&config, configFile))
		return true;
	return false;
}
