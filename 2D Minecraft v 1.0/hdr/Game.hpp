#pragma once

// Dependencies
#include "graphics/UI.hpp"
#include "graphics/lighting/Lighting.hpp"
#include "util/Loader.hpp"
#include "world/Generation.hpp"
#include "entity/Collision.hpp"
#include "entity/Spawn.hpp"
#include "audio/AudioEnviroment.hpp"
#include "world/PathFinding.hpp"

namespace game {
	extern const std::string gamePropertiesFileName; 

	extern gs::util::Clock timer; 

	void create(); 
	void update(); 
	void close(); 
	bool isOpen(); 
	void loadGameProperties(); 
	void saveGameProperties(); 

	void updateEvents(); 
	void updateGraphics(); 
}