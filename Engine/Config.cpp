#include "Config.hpp"

void Config::setAudioMasterVolume(int volume) {
	this->volume = volume;
}

int Config::getAudioMasterVolume() {
	return volume;
}

void Config::setGraphicsVsync(bool vsync) {
	this->vsync = vsync;
}

bool Config::getGraphicsVsync() {
	return vsync;
}

void Config::setGraphicsTripleBuffering(bool tripleBuffering) {
	this->tripleBuffering = tripleBuffering;
}

bool Config::getGraphicsTripleBuffering() {
	return tripleBuffering;
}

void Config::setGraphicsTextureQuality(Quality textureQuality) {
	this->textureQuality = textureQuality;
}

Quality Config::getGraphicsTextureQuality() {
	return textureQuality;
}

void Config::setGraphicsTextureFiltering(TextureFiltering textureFiltering) {
	this->textureFiltering = textureFiltering;
}

TextureFiltering Config::getGraphicsTextureFiltering() {
	return textureFiltering;
}

void Config::setWindowWidth(int width) {
	this->windowWidth = width;
}

int Config::getWindowWidth() {
	return windowWidth;
}

void Config::setWindowHeight(int height) {
	this->windowHeight = height;
}

int Config::getWindowHeight() {
	return windowHeight;
}

void Config::setWindowMode(WindowMode mode) {
	this->windowMode = mode;
}

WindowMode Config::getWindowMode() {
	return windowMode;
}
