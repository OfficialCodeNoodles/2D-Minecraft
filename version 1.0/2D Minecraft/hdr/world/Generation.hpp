#pragma once

// Dependencies
#include "Biome.hpp"
#include "Structure.hpp"
#include "Generator.hpp"
#include "PathFinding.hpp"

namespace engine {
	extern const int waterBodySpawnAttempts;
	extern const int waterBodyDistanceFromSurface; 
	extern const gs::Vec2i waterBodyVolumeRange;
	extern const int lavaBodySpawnAttempts;
	extern const int lavaBodyDistanceFromSurface; 
	extern const gs::Vec2i lavaBodyVolumeRange;
	extern const int maxFluidBodyStretch; 
	extern const int waterFallSpawnAttempts; 
	extern const int waterFallDistanceFromSurface; 
	extern const int lavaFallSpawnAttempts; 
	extern const int lavaFallDistanceFromSurface; 

	Biome::Id getBiome(int chunkOffset); 
	float getChunkStartXpos(int chunkOffset); 
	int* generateHeightMap(int chunkOffset); 
	std::vector<gs::Vec2i>& generateCircleBlocks(gs::Vec2i position, float radius); 

	void generateTree(gs::Vec2i position, TreeType treeType, World& world);
	void generateTree(int xpos, int ypos, TreeType treeType, World& world); 
	void generateFluidBody(PathFinder& pathFinder, Block::Id fluid, World& world); 
	void generateBiomeSpecificFeatures(
		Chunk& chunk, World& world, const int* heightMap
	); 
	void generateChunk(Chunk& chunk, World& world); 
}