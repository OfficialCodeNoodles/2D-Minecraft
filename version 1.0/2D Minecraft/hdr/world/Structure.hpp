#pragma once

// Dependencies
#include "World.hpp"

namespace engine {
	class Structure {
	public:
		enum Id { Dungeon, End } id;
		int spawnAttempts; 

		Structure(); 
		Structure(Structure::Id structureId); 
		Structure(Structure::Id structureId, int spawnAttempts); 
		~Structure() = default; 

		gs::Vec2i generateStructureLocation(
			Chunk& chunk, World& world, const int* heightMap, int bedrockOffset
		);
		void generateStructure(Chunk& chunk, World& world, gs::Vec2i chunkPosition); 

		static const gs::Vec2i dungeonSize; 
		static Structure structures[End]; 
	private:
		int spawnIterator; 
	};

	void generateStructures(Chunk& chunk, World& world, const int* heightMap);
}