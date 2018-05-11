#pragma once

#include "AudioManager.hpp"

class AudioManager {
public:
	AudioManager(UserAudioSettings settings);
private:
	UserAudioSettings settings;
};