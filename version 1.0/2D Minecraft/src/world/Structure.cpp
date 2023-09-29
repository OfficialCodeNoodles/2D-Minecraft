#include "../../hdr/world/Structure.hpp"
#include "../../hdr/util/Random.hpp"

namespace engine {
	Structure::Structure() : 
		id(Id::End),
		spawnAttempts(0),
		spawnIterator(0)
	{
	}
	Structure::Structure(Structure::Id structureId) : Structure() {
		id = structureId; 
	}
	Structure::Structure(Structure::Id structureId, int spawnAttempts) :
		Structure()
	{
		id = structureId; 
		this->spawnAttempts = spawnAttempts; 
	}

	gs::Vec2i Structure::generateStructureLocation(
		Chunk& chunk, World& world, const int* heightMap, int bedrockOffset)
	{
		static Random randomGenerator; 

		randomGenerator.setSeed(world.seed + chunk.offset); 

		const int verticalOffset = 10; 
		gs::Vec2i position = gs::Vec2i(-1, -1); 

		position.x = randomGenerator.generate() 
			% (Chunk::width - dungeonSize.x); 
		position.y = (randomGenerator.generate() 
			% (Chunk::height - heightMap[position.x] - verticalOffset 
				- bedrockOffset)) + heightMap[position.x] + verticalOffset; 

		spawnIterator++; 
		spawnIterator %= spawnAttempts; 

		return position;
	}

	void Structure::generateStructure(Chunk& chunk, World& world, gs::Vec2i chunkPosition) {
		static Random randomGenerator;
		
		// Global position of structure. 
		const gs::Vec2i position = gs::Vec2i(
			chunkPosition.x + (chunk.offset * Chunk::width), chunkPosition.y
		);

		randomGenerator.setSeed(chunk.offset); 

		switch (id) {
		case Dungeon:
		{
			// Generates the gradiant of cobblestone and mossy cobblestone that
			// border the dungeon. 
			auto generateDungeonBlock = []() -> Block::Id {
				return randomGenerator.generate() % 2 == 0 ? Block::Cobblestone
					: Block::MossyCobblestone; 
			}; 
			// Generates the gradiant of cobblesone and mossy cobblestone walls
			// that border the dungeon. 
			auto generateDungeonWall = []() -> Wall::Id {
				return randomGenerator.generate() % 2 == 0 ? Wall::Cobblestone
					: Wall::MossyCobblestone; 
			}; 

			for (int xoffset = 0; xoffset < dungeonSize.x; xoffset++) {
				// How far down the walls should go. Note: is only greater than
				// one on the edges. 
				const int blockDepth = xoffset == 0 || xoffset == 
					(dungeonSize.x - 1) ? dungeonSize.y : 1; 
				const gs::Vec2i roofPosition = gs::Vec2i(
					position.x + xoffset, position.y
				); 
				const gs::Vec2i floorPosition = gs::Vec2i(
					position.x + xoffset, position.y + dungeonSize.y
				);
				
				// Places block for roof if it isn't empty. 
				if (!world.getBlock(roofPosition).isEmpty())
					world.placeBlock(roofPosition, generateDungeonBlock());
				// Places block for floor if it isn't empty. 
				if (!world.getBlock(floorPosition).isEmpty())
					world.placeBlock(floorPosition, generateDungeonBlock()); 

				for (int yoffset = 1; yoffset < dungeonSize.y; yoffset++) {
					const gs::Vec2i offsetPosition = gs::Vec2i(
						position.x + xoffset, position.y + yoffset
					);

					if (yoffset < blockDepth) {
						if (!world.getBlock(offsetPosition).isEmpty())
							world.placeBlock(offsetPosition, generateDungeonBlock());
					}
					else {
						world.placeBlock(offsetPosition, Block::Air);
						world.placeWall(offsetPosition, generateDungeonWall());
					}
				}
			}

			// Position for spawner that is in the middle of the dungeon floor.
			const gs::Vec2i spawnerPosition = gs::Vec2i(
				position.x + (dungeonSize.x / 2),
				position.y + dungeonSize.y - 1
			);

			world.placeBlock(spawnerPosition, Block::Spawner); 

			// Randomly generated offset for chest within the dungeon. 
			const int initialChestSpawnXoffset = randomGenerator.generate()
				% (dungeonSize.x - 2); 

			gs::Vec2i chestPostion = gs::Vec2i(
				initialChestSpawnXoffset, dungeonSize.y - 1
			); 

			do {
				const gs::Vec2i translatedChestPostion = position 
					+ chestPostion + gs::Vec2i(1, 0); 

				// Only places chest if the block it will occupy is empty, and
				// the block beneath is solid. 
				if (world.getBlock(translatedChestPostion).isEmpty()
					&& !world.getBlock(translatedChestPostion 
						+ gs::Vec2i(0, 1)).isEmpty())
				{
					world.placeBlock(translatedChestPostion, Block::Chest); 
					
					Block& chest = world.getBlockRef(translatedChestPostion); 
					// Sets the chest's loot to be generated later. 
					chest.tags.lootTable = LootTable::DungeonChestLoot;

					break; 
				}

				chestPostion.x++; 
				chestPostion.x %= dungeonSize.x - 2;
			} 
			while (chestPostion.x != initialChestSpawnXoffset); 
		}
			break; 
		}
	}

	const gs::Vec2i Structure::dungeonSize = gs::Vec2i(11, 7);
	Structure Structure::structures[End] = { 
		{ Structure::Dungeon, 1 }
	};

	void generateStructures(Chunk& chunk, World& world, const int* heightMap) {
		for (auto& structure : Structure::structures) {
			for (int spawnAttempt = 0; spawnAttempt < structure.spawnAttempts;
				spawnAttempt++)
			{
				// Generated location for structure. 
				const gs::Vec2i structurePosition =
					structure.generateStructureLocation(
						chunk, world, heightMap, 20
					); 

				// Only generate structure if a valid position was returned. 
				if (structurePosition != gs::Vec2i(-1, -1))
					structure.generateStructure(chunk, world, structurePosition);
			}
		}
	}
}