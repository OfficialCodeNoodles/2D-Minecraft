#pragma once

// Dependencies
#include "Tile.hpp"

namespace engine {
	extern class Generator; 

	enum class TreeType { 
		None, Oak, Birch, Cactus, Spruce, Jungle, Acacia, End 
	};

	extern const Block::Id logTypes[static_cast<int>(TreeType::End)]; 
	extern const Block::Id leaveTypes[static_cast<int>(TreeType::End)]; 
	extern const gs::Vec2i treeHeightRanges[static_cast<int>(TreeType::End)]; 
	extern const int treeTopLeaveCounts[static_cast<int>(TreeType::End)]; 

	struct Biome {
		enum Id { 
			Plains,					Forest, 
			BirchForest,			Desert,
			Taiga,					Jungle, 
			Mountain,				Swamp, 
			Savanna, 
			End
		} id;

		Biome();
		Biome(Biome::Id biomeId);
		~Biome() = default;

		static const std::string biomeStrings[End];
	};

	class BiomeInfo {
	public:
		std::vector<Generator> generators; 
		gs::Color color; 
		float terrainFrequency; 
		float terrainAmplitude; 
		TreeType treeType; 
		float treeFrequency; 
		int rate;  

		BiomeInfo(); 
		~BiomeInfo() = default; 
 
		static constexpr int biomeChunkSize = 6; 
		static BiomeInfo biomeInfo[Biome::End]; 
	private:
	};

	void loadBiomeInfo(); 
}  