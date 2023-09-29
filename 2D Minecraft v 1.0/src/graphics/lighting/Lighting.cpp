#include "../../../hdr/graphics/lighting/Lighting.hpp"
#include "../../../hdr/entity/Spawn.hpp"

namespace engine {
	namespace render {
		namespace lighting {
			int lightUpdateRate = 10;
			bool forceLights = false;
			std::vector<std::pair<Light::Id, gs::Vec2i>> lights;
			TileColor ambientLightColor = TileColor(8, 9, 12);
			TileColor sunlightColor;
			TileColor moonlightColor = TileColor(50, 60, 80);
			float sunlightBrightness = 100.0f;
			LightingStyle lightingStyle = LightingStyle::Geometry;
			int lightsRendered = 0; 
			bool fullBrightEnabled = false; 
			int ticksUntilNextLightingUpdate = 0;
			int ticksUntilNextSpawningLightingUpdate = 
				timeBetweenSpawnLightingUpdates;

			bool isValidLight(gs::Vec2i position, Light::Id lightId) {
				// Translations of all cardinal neighbors. 
				const gs::Vec2i neighbors[4] = {
					gs::Vec2i(0, -1), gs::Vec2i(1, 0),
					gs::Vec2i(0, 1), gs::Vec2i(-1, 0)
				};

				for (auto& neighbor : neighbors) {
					const Block neighborBlock =
						world->getBlock(position + neighbor);
					const Wall neighborWall =
						world->getWall(position + neighbor);

					if (BlockInfo::getVar(neighborBlock.id,
						BlockInfo::lightIndex) != lightId
							|| !neighborWall.isEmpty())
						return true;
				}

				return false;
			}
			TileColor maximizeColors(TileColor color0, TileColor color1) {
				return TileColor(
					std::max(color0.r, color1.r),
					std::max(color0.g, color1.g),
					std::max(color0.b, color1.b)
				);
			}
			bool colorGreaterThan(TileColor color0, TileColor color1) {
				return color0.r > color1.r && color0.g > color1.g 
					&& color0.b > color1.b;
			}
			void applyLight(const Light& lightSource, gs::Vec2i position, World& world) {
				// Calculates distance as the sum of the absolute difference of 
				// it's components. Creates a diamond distrabution. 
				auto manhattan = [](gs::Vec2i start, gs::Vec2i end) -> int {
					return std::abs(start.x - end.x) 
						+ std::abs(start.y - end.y); 
				}; 

				const int centerRadius = lightingStyle == LightingStyle::Geometry
					? lightSource.centerRadius : lightSource.centerRadius + 1; 
				const int size = lightSource.strength + centerRadius; 
				const gs::Vec2i startPosition = position - gs::Vec2i(size - 2, 0);
				const TileColor lightColor = lightSource.getColor(position); 

				// Vertical distance away from the center. 
				int yDelta = 0; 

				for (int xpos = startPosition.x; xpos < position.x + size; xpos++) {
					for (int ypos = startPosition.y - yDelta; ypos <
						startPosition.y + yDelta + 1; ypos++)
					{
						// Distance between current tile and the center of the light.
						const float distance = manhattan({ xpos, ypos }, position);

						// Brightness ratio of tile, based on it's distance. 
						float ratio = std::max(distance - centerRadius, 0.0f) 
							/ static_cast<float>(lightSource.strength - 1);

						if (lightingStyle == LightingStyle::Smooth)
							ratio = std::sqrt(ratio);
						
						const TileColor tileColor = gs::util::approach(
							xpos == position.x && ypos == position.y 
								&& lightSource.flickerMaintainCenter 
									? lightSource.baseColor : lightColor, 
							ambientLightColor, 
							std::min(ratio * 100.0f, 100.0f)
						);
						const TileColor baseColor = world.getTileColor(xpos, ypos); 

						world.setTileColor(xpos, ypos, maximizeColors(baseColor, tileColor));
					}

					// Creates the diamond shape. 
					yDelta += xpos < position.x ? 1 : -1; 
				}

				lightsRendered++; 
			}
			void attemptLight(gs::Vec2i	position) {
				const Block block = world->getBlock(position);
				const Wall wall = world->getWall(position);
				const Light::Id lightId = static_cast<Light::Id>(
					block.getVar(BlockInfo::applyAnimationLighting)
					? Block(static_cast<Block::Id>(block.id
						+ block.tags.animationOffset)).getVar(
							BlockInfo::lightIndex)
							: block.getVar(BlockInfo::lightIndex)
				);

				TileColor tileColor = ambientLightColor;

				if (lightId != Light::None && (wall.isEmpty()
					|| lightId != Light::Sunlight))
				{
					const Light& lightSource =
						Light::lightSources[lightId];

					tileColor = lightSource.getColor(position);

					if (isValidLight(position, lightId)) {
						lights.push_back(
							std::pair<Light::Id, gs::Vec2i>(lightId, position)
						);

						world->setTileColor(position, ambientLightColor);
						return; 
					}
				}

				world->setTileColor(position, tileColor);
			}
			void updateWorldLights(World& world) {
				// Exits function if full bright is enabled. 
				if (fullBrightEnabled) {
					lightsRendered = 0;
					return; 
				}

				// Calculates how often the lights should be updated based on 
				// the number of lights rendered in the prvs frame. 
				lightUpdateRate = std::min((lightsRendered / 30) + 2, 20); 

				if (forceLights) {
					ticksUntilNextLightingUpdate = 0;
					forceLights = false;
				}

				if (ticksUntilNextSpawningLightingUpdate == 0) {
					updateSpawningLights(world); 

					ticksUntilNextLightingUpdate = lightUpdateRate; 
					ticksUntilNextSpawningLightingUpdate = 
						timeBetweenSpawnLightingUpdates; 
				}

				if (ticksUntilNextLightingUpdate == 0) {
					lights.clear(); 
					lightsRendered = 0; 

					for (int xpos = renderableHorizontalLightRange.x; xpos <
						renderableHorizontalLightRange.y; xpos++)
					{
						for (int ypos = renderableVerticalLightRange.x; ypos <
								renderableVerticalLightRange.y; ypos++)
							attemptLight({ xpos, ypos }); 
					}
					
					// Applies all current light sources. 
					for (auto& [lightSourceIndex, position] : lights) {
						const Light& lightSource = 
							Light::lightSources[lightSourceIndex]; 

						applyLight(lightSource, position, world); 
					}

					ticksUntilNextLightingUpdate = lightUpdateRate;
				}

				ticksUntilNextLightingUpdate--;
				ticksUntilNextSpawningLightingUpdate--; 
			}
			void updateSpawningLights(World& world) {
				lights.clear(); 

				for (int xpos = renderableHorizontalSpawningLightRange.x;
					xpos < renderableHorizontalSpawningLightRange.y; xpos++)
				{
					for (int ypos = renderableVerticalSpawningLightRange.x;
							ypos < renderableVerticalSpawningLightRange.y; ypos++)
						attemptLight({ xpos, ypos });
				}

				// Applies all current light sources. 
				for (auto& [lightSourceIndex, position] : lights) {
					const Light& lightSource =
						Light::lightSources[lightSourceIndex];

					applyLight(lightSource, position, world);
					// Doesn't count these lights as part of the total. 
					lightsRendered--; 
				}
			}
		}
	}
}