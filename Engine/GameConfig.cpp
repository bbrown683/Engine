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

#include "GameConfig.hpp"

/*

GameConfig::GameConfig() {
    config_init(&config);
    root = config_root_setting(&config);
    audio = config_setting_add(root, "audio", CONFIG_TYPE_GROUP);
    graphics = config_setting_add(root, "graphics", CONFIG_TYPE_GROUP);
    window = config_setting_add(root, "window", CONFIG_TYPE_GROUP);
    m_Vsync = config_setting_add(graphics, "vsync", CONFIG_TYPE_BOOL);
    textureQuality = config_setting_add(graphics, "textureQuality", CONFIG_TYPE_INT);
    m_TextureFiltering = config_setting_add(graphics, "textureFiltering", CONFIG_TYPE_INT);
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
    graphicsSettings.m_Vsync = true;
    config_setting_set_bool(m_Vsync, graphicsSettings.m_Vsync);
    graphicsSettings.textureQuality = Quality::Low;
    config_setting_set_int(textureQuality, static_cast<int>(graphicsSettings.textureQuality));
    graphicsSettings.m_TextureFiltering = TextureFiltering::None;
    config_setting_set_int(m_TextureFiltering, static_cast<int>(graphicsSettings.m_TextureFiltering));
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

bool GameConfig::setGraphicsVsync(bool m_Vsync) {
    graphicsSettings.m_Vsync = m_Vsync;
    config_setting_set_bool(this->m_Vsync, graphicsSettings.m_Vsync);
    return writeConfig();
}

bool GameConfig::setGraphicsTextureQuality(Quality textureQuality) {
    graphicsSettings.textureQuality = textureQuality;
    config_setting_set_int(this->textureQuality, static_cast<int>(graphicsSettings.textureQuality));
    return writeConfig();
}

bool GameConfig::setGraphicsTextureFiltering(TextureFiltering m_TextureFiltering) {
    graphicsSettings.m_TextureFiltering = m_TextureFiltering;
    config_setting_set_int(this->m_TextureFiltering, static_cast<int>(graphicsSettings.m_TextureFiltering));
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

*/