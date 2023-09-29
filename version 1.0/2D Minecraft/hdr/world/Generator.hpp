#pragma once

// Dependencies
#include "World.hpp"
#include "../util/Random.hpp"
#include "../../vendor/FractalNoise.h"

namespace engine {
	class Generator {
	public:
		using PlaceFilter = World::PlaceFilter; 

		// Determines what vertical range should be used for generator. 
		enum class RangeFilter {
			HeightMap, // Start from the height-map
			TopDown, // Start from top, y=0
			BottomUp // Start from bottom
		} rangeFilter;
		union { int start; int offset; };
		union { int end; int distance; }; 
		// Effects the condition required to place a block. 
		enum class ConditionalFilter {
			Solid, // Always place block
			Noise // Use perlin noise
		} conditionalFilter;
		float noiseFrequency;
		int noiseOctaves; 
		float noiseLowerBound; 
		float noiseUpperBound; 
		float noiseHeightScaler; 
		// Effects if the upper/lower edges of the generation fade out using 
		// noise. 
		enum class EdgeNoiseFilter {
			None,
			Top = 1,
			Bottom = 1 << 1, 
			Dual = Top | Bottom
		} edgeNoiseFilter;
		int edgeNoiseOffset; 
		bool isPlant; 
		PlaceFilter placeFilter; 
		Block::Id blockId;
		Wall::Id wallId; 

		bool useBlock; 
		int layer; 

		Generator(); 
		~Generator() = default; 

		static constexpr int numOfLayers = 5;
		static constexpr int middleLayer = numOfLayers / 2; 
	private:
	};

	extern const float seedScaler; 

	extern std::vector<Generator> defaultGenerators; 
	extern int generatorSeed;

	void loadGenerators(
		std::vector<Generator>& generators, const std::string& filename
	); 
	void loadDefaultGenerators(); 
	void applyGenerator(
		const Generator& generator, Chunk& chunk, World& world,
		const int* heightMap, int generatorIndex = 0
	); 
}