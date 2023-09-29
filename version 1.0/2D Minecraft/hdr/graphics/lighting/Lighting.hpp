#pragma once

// Dependencies
#include "Light.hpp"
#include "../Render.hpp"
#include "../../world/World.hpp"

namespace engine {
	namespace render {
		namespace lighting {
			enum class LightingStyle { Geometry, Smooth };
			constexpr int timeBetweenSpawnLightingUpdates = 120; 

			extern int lightUpdateRate;
			extern bool forceLights;
			extern std::vector<std::pair<Light::Id, gs::Vec2i>> lights; 
			extern TileColor ambientLightColor;
			extern TileColor sunlightColor; 
			extern TileColor moonlightColor;
			extern float sunlightBrightness; 
			extern LightingStyle lightingStyle;
			extern int lightsRendered; 
			extern bool fullBrightEnabled; 
			extern int ticksUntilNextLightingUpdate;
			extern int ticksUntilNextSpawningLightingUpdate;

			bool isValidLight(gs::Vec2i position, Light::Id lightId); 
			TileColor maximizeColors(TileColor color0, TileColor color1); 
			bool colorGreaterThan(TileColor color0, TileColor color1); 
			void applyLight(const Light& lightSource, gs::Vec2i position, World& world);
			void attemptLight(gs::Vec2i	position);
			void updateWorldLights(World& world);
			void updateSpawningLights(World& world); 
		}
	}
}