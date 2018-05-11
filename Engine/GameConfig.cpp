#include "GameConfig.hpp"

GameConfig::GameConfig() {
	config_init(&config);
	
}

GameConfig::~GameConfig() {
	config_destroy(&config);
}
