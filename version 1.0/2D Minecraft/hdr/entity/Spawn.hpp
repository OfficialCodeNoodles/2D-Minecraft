#pragma once

// Dependencies
#include "Mob.hpp"

namespace engine {
	extern const gs::Vec2i minSpawningDistance; 
	extern const gs::Vec2i maxSpawningDistance; 
	extern const float minMobDistance; 

	struct SpawnInfo {
		Mob::Type mobType; 
		int spawnAttempts; 
		Biome::Id biomeRequired; 

		SpawnInfo(); 
		~SpawnInfo() = default; 

		static constexpr int numOfMobSpawners = 10; 
		static SpawnInfo spawnInfo[numOfMobSpawners]; 
	};

	extern gs::Vec2i spawingOrigin; 

	void loadSpawnInfo(); 
	bool isValidSpawningPosition(
		gs::Vec2i position, gs::Vec2f size, Mob::Behavior mobBehavior
	); 
	void updateMobSpawning(); 
}