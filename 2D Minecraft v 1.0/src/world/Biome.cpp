#include "../../hdr/world/Biome.hpp"
#include "../../hdr/world/Generator.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	const Block::Id logTypes[static_cast<int>(TreeType::End)] = {
		Block::Air, Block::OakLog, Block::BirchLog, Block::Cactus, 
		Block::SpruceLog, Block::JungleLog,	Block::AcaciaLog
	}; 
	const Block::Id leaveTypes[static_cast<int>(TreeType::End)] = {
		Block::Air, Block::OakLeaves, Block::BirchLeaves, Block::Air, 
		Block::SpruceLeaves, Block::JungleLeaves, Block::AcaciaLeaves
	}; 
	const gs::Vec2i treeHeightRanges[static_cast<int>(TreeType::End)] = {
		gs::Vec2i(), gs::Vec2i(4, 7), gs::Vec2i(5, 9), gs::Vec2i(1, 3),
		gs::Vec2i(8, 15), gs::Vec2i(4, 15), gs::Vec2i(6, 12)
	}; 
	const int treeTopLeaveCounts[static_cast<int>(TreeType::End)] = {
		0, 2, 2, 0, 1, 2, 2
	}; 

	Biome::Biome() : id(Biome::Id::Plains) {
	}
	Biome::Biome(Biome::Id biomeId) : id(biomeId) {
	}

	const std::string Biome::biomeStrings[End] = {
		"plains",					"forest", 
		"birchForest",				"desert",
		"taiga",					"jungle", 
		"mountain",					"swamp",
		"savanna"
	};

	BiomeInfo::BiomeInfo() : 
		color(gs::Color(0, 255, 20)), 
		terrainFrequency(1.0f), 
		terrainAmplitude(40.0f), 
		treeType(TreeType::None), 
		treeFrequency(1.0f), 
		rate(10) 
	{
	}

	BiomeInfo BiomeInfo::biomeInfo[Biome::End]; 

	void loadBiomeInfo() {
		for (int biomeIndex = 0; biomeIndex < Biome::End; biomeIndex++) {
			const std::string filename = render::assetDirectory + "data/biomes/"
				+ Biome::biomeStrings[biomeIndex] + ".list"; 
			const PairVector& pairs = loadPairedFile(
				filename, '=', "EndProperties");

			BiomeInfo& biomeInfo = BiomeInfo::biomeInfo[biomeIndex]; 

			for (auto& [attribute, value] : pairs) {
				if (attribute == "redValue")
					biomeInfo.color.r = std::stoi(value); 
				else if (attribute == "greenValue")
					biomeInfo.color.g = std::stoi(value);
				else if (attribute == "blueValue")
					biomeInfo.color.b = std::stoi(value);
				else if (attribute == "terrainFrequency")
					biomeInfo.terrainFrequency = std::stof(value);
				else if (attribute == "terrainAmplitude")
					biomeInfo.terrainAmplitude = std::stof(value);
				else if (attribute == "treeType")
					biomeInfo.treeType = static_cast<TreeType>(std::stoi(value));
				else if (attribute == "treeFrequency")
					biomeInfo.treeFrequency = std::stof(value); 
				else if (attribute == "rate")
					biomeInfo.rate = std::stoi(value);
			}

			loadGenerators(biomeInfo.generators, filename);
		}
	}
}