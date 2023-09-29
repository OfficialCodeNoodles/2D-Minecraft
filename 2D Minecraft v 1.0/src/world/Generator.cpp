#include "../../hdr/world/Generator.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	Generator::Generator() : 
		rangeFilter(RangeFilter::HeightMap), 
		conditionalFilter(ConditionalFilter::Solid), 
		noiseFrequency(1.0f),
		noiseOctaves(3), 
		noiseLowerBound(0.0f), 
		noiseUpperBound(0.1f), 
		noiseHeightScaler(0.0f),
		placeFilter(PlaceFilter::Replace),
		edgeNoiseFilter(EdgeNoiseFilter::None),
		edgeNoiseOffset(0), 
		isPlant(false), 
		useBlock(true), 
		layer(middleLayer)
	{
	}

	void applyGenerator(
		const Generator& generator, Chunk& chunk, World& world, 
		const int* heightMap, int generatorIndex)
	{
		static FractalNoise noiseGenerator; 

		const float generatorOffset = generatorIndex * 100.0f; 
		const float seedOffset = generatorSeed * seedScaler;

		Random edgeNoiseValueGenerator(seedOffset); 

		gs::Vec2i chunkPosition; 
		gs::Vec2i verticalRange; 

		if (generator.conditionalFilter == Generator::ConditionalFilter::Noise) {
			// Initialize perlin noise generator. 
			noiseGenerator.setBaseFrequency(1.0f / generator.noiseFrequency);
			noiseGenerator.setOctaves(generator.noiseOctaves); 
		}

		if (generator.rangeFilter == Generator::RangeFilter::TopDown) {
			verticalRange.x = generator.start;
			verticalRange.y = generator.end != -1 ? generator.end 
				: Chunk::height; 
		}
		else if (generator.rangeFilter == Generator::RangeFilter::BottomUp) {
			verticalRange.x = Chunk::height - (generator.end != -1 
				? generator.end : Chunk::height); 
			verticalRange.y = Chunk::height - generator.start; 
		}

		for (; chunkPosition.x < Chunk::width; chunkPosition.x++) {
			if (generator.rangeFilter == Generator::RangeFilter::HeightMap) {
				verticalRange.x = heightMap[chunkPosition.x] + generator.offset; 
				verticalRange.y = generator.distance != -1 ? verticalRange.x +
					generator.distance : Chunk::height;

				gs::util::clamp(&verticalRange.x, 0, Chunk::height); 
				gs::util::clamp(&verticalRange.y, 0, Chunk::height); 
			}

			for (chunkPosition.y = verticalRange.x; 
				chunkPosition.y < verticalRange.y; chunkPosition.y++)
			{
				const gs::Vec2i worldPosition = gs::Vec2i(
					chunkPosition.x + (chunk.offset * Chunk::width),
					chunkPosition.y
				); 

				if (generator.edgeNoiseOffset != 0) {
					const int edgeNoiseFilter = static_cast<int>(generator.edgeNoiseFilter); 
					const int topFilter = static_cast<int>(Generator::EdgeNoiseFilter::Top); 
					const int bottomFilter = static_cast<int>(Generator::EdgeNoiseFilter::Bottom);

					// Creates edge noise gradiant. 
					if ((edgeNoiseFilter & topFilter) == topFilter) {
						const int topDelta = chunkPosition.y - verticalRange.x;

						if (topDelta < generator.edgeNoiseOffset) {
							const float percentage = (static_cast<float>(topDelta)
								/ static_cast<float>(generator.edgeNoiseOffset)) * 100.0f;

							if (edgeNoiseValueGenerator.generate() % 100 
									> static_cast<int>(percentage))
								continue;
						}
					}
					if ((edgeNoiseFilter & bottomFilter) == bottomFilter) {
						const int bottomDelta = verticalRange.y - chunkPosition.y;

						if (bottomDelta < generator.edgeNoiseOffset) {
							const float percentage = (static_cast<float>(bottomDelta)
								/ static_cast<float>(generator.edgeNoiseOffset)) * 100.0f;

							if (edgeNoiseValueGenerator.generate() % 100 
									> static_cast<int>(percentage))
								continue;
						}
					}
				}

				if (generator.conditionalFilter == Generator::ConditionalFilter::Noise) {
					const float noiseValue = noiseGenerator.noise(
						worldPosition.x + generatorOffset,
						chunkPosition.y + generatorOffset,
						seedOffset
					); 

					const float heightRatio = static_cast<float>(worldPosition.y 
						- verticalRange.x) / static_cast<float>(std::max(verticalRange.y 
							- verticalRange.x, 1)); 
					const float noiseHeightModifier = gs::util::approach(
						0.0f, generator.noiseHeightScaler, heightRatio * 100.0f); 

					const float lowerBound = generator.noiseLowerBound - noiseHeightModifier;
					const float upperBound = generator.noiseUpperBound + noiseHeightModifier;

					if (noiseValue < lowerBound || noiseValue > upperBound)
						continue; 
				}

				if (generator.isPlant) {
					const Block blockBeneath = world.getBlock(
						worldPosition + gs::Vec2i(0, 1)); 

					// Makes sure plant is placed in a valid location. 
					if (!world.isValidBlockPlacementLocation(worldPosition, 
							generator.blockId))
						continue; 
				}

				if (generator.useBlock) [[likely]] {
					Block generatedBlock(generator.blockId); 
					generatedBlock.tags.naturalBlock = true; 

					world.placeBlock(worldPosition, generatedBlock, 
						generator.placeFilter);
				}
				else {
					Wall generatedWall(generator.wallId); 

					world.placeWall(worldPosition, generatedWall,
						generator.placeFilter);
				}
			}
		}
	}

	const float seedScaler = 100.0f; 

	std::vector<Generator> defaultGenerators; 
	int generatorSeed = 0; 

	void loadGenerators(
		std::vector<Generator>& generators, const std::string& filename) 
	{
		const PairVector& pairs = loadPairedFile(filename);

		Generator* generator = nullptr; 
		int generatorLayer = 1; 

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewGenerator") {
				if (generator != nullptr) {
					generator->layer = generatorLayer; 
					generators.push_back(*generator);
					delete generator; 
				}

				generator = new Generator(); 
			}
			else if (generator != nullptr) {
				if (attribute == "layer")
					generatorLayer = std::stoi(value);
				else if (attribute == "blockId") {
					generator->blockId = static_cast<Block::Id>(std::stoi(value));
					generator->useBlock = true;
				}
				else if (attribute == "wallId") {
					generator->wallId = static_cast<Wall::Id>(std::stoi(value));
					generator->useBlock = false;
				}
				else if (attribute == "rangeFilter")
					generator->rangeFilter = static_cast<Generator::RangeFilter>(
						std::stoi(value));
				else if (attribute == "start" || attribute == "offset")
					generator->start = std::stoi(value);
				else if (attribute == "end" || attribute == "distance")
					generator->end = std::stoi(value);
				else if (attribute == "conditionalFilter")
					generator->conditionalFilter = static_cast<Generator::ConditionalFilter>(
						std::stoi(value));
				else if (attribute == "noiseFrequency")
					generator->noiseFrequency = std::stof(value);
				else if (attribute == "noiseOctaves")
					generator->noiseOctaves = std::stoi(value);
				else if (attribute == "noiseLowerBound")
					generator->noiseLowerBound = std::stof(value);
				else if (attribute == "noiseUpperBound")
					generator->noiseUpperBound = std::stof(value);
				else if (attribute == "noiseHeightScaler")
					generator->noiseHeightScaler = std::stof(value);
				else if (attribute == "edgeNoiseFilter")
					generator->edgeNoiseFilter = static_cast<Generator::EdgeNoiseFilter>(
						std::stoi(value));
				else if (attribute == "edgeNoiseOffset")
					generator->edgeNoiseOffset = std::stoi(value);
				else if (attribute == "isPlant")
					generator->isPlant = static_cast<bool>(std::stoi(value)); 
				else if (attribute == "placeFilter")
					generator->placeFilter = static_cast<Generator::PlaceFilter>(
						std::stoi(value));
			}
		}

		if (generator != nullptr) {
			generator->layer = generatorLayer; 
			generators.push_back(*generator); 
			delete generator; 
		}
	}
	void loadDefaultGenerators() {
		loadGenerators(
			defaultGenerators, render::assetDirectory + "data/generator.list"
		);
	}
}