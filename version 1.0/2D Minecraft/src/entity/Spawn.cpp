#include "../../hdr/entity/Spawn.hpp"
#include "../../hdr/util/Random.hpp"
#include "../../hdr/graphics/lighting/Lighting.hpp"
#include "../../hdr/entity/Collision.hpp"
#include "../../hdr/world/World.hpp"

namespace engine {
	const gs::Vec2i minSpawningDistance = gs::Vec2i(
		((render::window::defaultWindowWidth / 1.5f)//render::minCameraScale)
			/ render::tileSize) / 2,
		((render::window::defaultWindowHeight / 1.5f)//render::minCameraScale)
			/ render::tileSize) / 2
	);
	const gs::Vec2i maxSpawningDistance = gs::Vec2i(
		((render::window::defaultWindowWidth / render::minCameraScale)
			/ render::tileSize) / 2,
		((render::window::defaultWindowHeight / render::minCameraScale)
			/ render::tileSize) / 2
	); 
	const float minMobDistance = 5.0f; 

	SpawnInfo::SpawnInfo() :
		mobType(Mob::Type::Zombie), 
		spawnAttempts(0),
		biomeRequired(Biome::End)
	{
	}

	SpawnInfo SpawnInfo::spawnInfo[numOfMobSpawners];

	gs::Vec2i spawingOrigin = gs::Vec2i(-1, -1);

	void loadSpawnInfo() {
		const PairVector& pairs = loadPairedFile(
			render::assetDirectory + "data/entity/spawn.list"
		);

		Mob::Type mobType = static_cast<Mob::Type>(0); 
		SpawnInfo* spawnInfo = nullptr;

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewSpawn") {
				mobType = static_cast<Mob::Type>(std::stoi(value));
				spawnInfo = &SpawnInfo::spawnInfo[mobType]; 
			}
			else if (spawnInfo != nullptr && !value.empty()) {
				if (attribute == "mobType")
					spawnInfo->mobType = static_cast<Mob::Type>(
						std::stoi(value)
					);
				else if (attribute == "spawnAttempts")
					spawnInfo->spawnAttempts = std::stoi(value);
			}
		}
	}
	bool isValidSpawningPosition(
		gs::Vec2i position, gs::Vec2f size, Mob::Behavior mobBehavior) 
	{
		const gs::Vec2i blockCheckingRange = gs::Vec2i(
			std::ceil(size.x / 2.0f) + 1, std::ceil(size.y) + 1
		);

		for (int ypos = position.y; ypos > position.y - blockCheckingRange.y; 
			ypos--) 
		{
			const bool groundBlock = ypos == position.y;

			for (int xpos = position.x - blockCheckingRange.x; xpos
				< position.x + blockCheckingRange.x; xpos++)
			{
				const Block block = world->getBlock(xpos, ypos);
				const TileColor tileColor = world->getTileColor(xpos, ypos);
				const collision::CollisionType collisionType =
					static_cast<collision::CollisionType>(block.getVar(
						BlockInfo::collisionType));

				if (groundBlock) {
					if (collisionType == collision::CollisionType::Block) {
						if (mobBehavior == Mob::Behavior::Hostile) {
							if (render::lighting::colorGreaterThan(tileColor,
									render::lighting::moonlightColor))
								return false; 
						}
						else {
							if (!world->isBlockExposedToSky(xpos, ypos - 1))
								return false; 
						}
					}
					else 
						return false; 
				}
				else {
					// Ensures the blocks the mob occupies aren't solid. 
					if (collisionType == collision::CollisionType::Block)
						return false;
				}
			}
		}

		return true;
	}
	void updateMobSpawning() {
		static Random randomGenerator;

		// Don't spawn anything if the spawn origin hasn't been established. 
		if (spawingOrigin == gs::Vec2i(-1, -1))
			return; 

		for (auto& spawnInfo : SpawnInfo::spawnInfo) {
			const Mob::Behavior mobBehavior = 
				Mob::mobBehaviors[static_cast<int>(spawnInfo.mobType)]; 

			// Ensures the mob caps haven't been exceeded before spawning mobs. 
			if (mobBehavior == Mob::Behavior::Hostile 
					&& Mob::numOfHostileMobs >= Mob::hostileMobSpawnCap)
				continue;
			else if (mobBehavior == Mob::Behavior::Passive
					&& Mob::numOfPassiveMobs >= Mob::passiveMobSpawnCap)
				continue; 

			for (int spawnAttempt = 0; spawnAttempt < spawnInfo.spawnAttempts;
				spawnAttempt++)
			{
				const gs::Vec2i offset = gs::Vec2i(
					minSpawningDistance.x + (randomGenerator.generate()
						% (maxSpawningDistance.x - minSpawningDistance.x)),
					minSpawningDistance.y + (randomGenerator.generate()
						% (maxSpawningDistance.y - minSpawningDistance.y))
				);

				gs::Vec2i spawningPositionOffset; 

				// Searches for a spawning position that isn't too close to the
				// player. 
				do {
					spawningPositionOffset.x = -maxSpawningDistance.x 
						+ (randomGenerator.generate()
							% (maxSpawningDistance.x * 2));
					spawningPositionOffset.y = -maxSpawningDistance.y
						+ (randomGenerator.generate()
							% (maxSpawningDistance.y * 2));
				} 
				while (std::abs(spawningPositionOffset.x) 
						< minSpawningDistance.x 
					&& std::abs(spawningPositionOffset.y) 
						< minSpawningDistance.y); 

				gs::Vec2i spawningPosition = spawingOrigin
					+ spawningPositionOffset;
				gs::util::clamp(&spawningPosition.y, 0, Chunk::height);

				const gs::Vec2f mobSize = Mob::mobSizes[
					static_cast<int>(spawnInfo.mobType)
				]; 
				const Mob::Type mobType = spawnInfo.mobType; 
				const Mob::Behavior mobBehavior = Mob::mobBehaviors[mobType]; 

				// If the spot selected is valid, a new mob is spawned. 
				if (isValidSpawningPosition(
					spawningPosition, mobSize, mobBehavior)) 
				{
					float lowestMobDistance = 1000.0f; 

					// Finds closest mob. 
					for (auto& [entityType, entity] : entities) {
						if (entityType == Entity::Type::Mob) {
							const Mob* mobEntity = dynamic_cast<Mob*>(entity);

							// Makes sure behavior matches mob being spawned. 
							if (mobEntity->behavior == mobBehavior) {
								lowestMobDistance = std::min(gs::util::distance(
									mobEntity->position, 
									gs::Vec2f(spawningPosition)
								), lowestMobDistance); 

								// Stops searching for closer mob once the min
								// spawn distance is reached. 
								if (lowestMobDistance < minMobDistance)
									break; 
							}
						}
					}

					// Only spawns mob if it wouldn't be too close to other 
					// mobs.
					if (lowestMobDistance >= minMobDistance) {
						Mob* newMob = new Mob;

						// Sets mob position to be standing on the center of 
						// the selected block. 
						newMob->position.x = spawningPosition.x
							+ (mobSize.x / 2.0f);
						newMob->position.y = spawningPosition.y
							- (mobSize.y / 2.0f);
						newMob->type = mobType;

						addEntity(Entity::Type::Mob, newMob);
					}
				}
			}
		}
	}
}