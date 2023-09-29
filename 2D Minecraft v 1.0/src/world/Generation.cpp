#include "../../hdr/world/Generation.hpp"

namespace engine {
	const int waterBodySpawnAttempts = 10;
	const int waterBodyDistanceFromSurface = -5;
	const gs::Vec2i waterBodyVolumeRange = gs::Vec2i(30, 300);
	const int lavaBodySpawnAttempts = 5;
	const int lavaBodyDistanceFromSurface = -5;
	const gs::Vec2i lavaBodyVolumeRange = gs::Vec2i(50, 500);
	const int maxFluidBodyStretch = 16;
	const int waterFallSpawnAttempts = 10;
	const int waterFallDistanceFromSurface = 10;
	const int lavaFallSpawnAttempts = 5;
	const int lavaFallDistanceFromSurface = 10;

	Biome::Id getBiome(int chunkOffset) {
		const int seed = (chunkOffset / BiomeInfo::biomeChunkSize)
			+ (generatorSeed * seedScaler); 

		//return Biome::Savanna; 
		//return static_cast<Biome::Id>(std::abs(chunkOffset) % Biome::End); 

		Random biomeValueGenerator(seed); 

		int rates[Biome::End];
		int rateOffset = 0; 

		for (int biomeIndex = 0; biomeIndex < Biome::End; biomeIndex++) {
			const BiomeInfo& biomeInfo = BiomeInfo::biomeInfo[biomeIndex]; 

			rateOffset += biomeInfo.rate; 
			rates[biomeIndex] = rateOffset; 
		}

		const int choosenIndex = biomeValueGenerator.generate() 
			% (rateOffset + 1);

		for (int rateIndex = 0; rateIndex < Biome::End; rateIndex++) {
			if (choosenIndex <= rates[rateIndex])
				return static_cast<Biome::Id>(rateIndex); 
		}
	}
	float getChunkStartXpos(int chunkOffset) {
		const bool negative = chunkOffset < 0; 
		const gs::Vec2i chunkRange = gs::Vec2i(
			negative ? chunkOffset : 0,
			negative ? 0 : chunkOffset
		); 

		float xpos = 0.0f;

		for (int chunkIndex = chunkRange.x; chunkIndex < chunkRange.y; 
			chunkIndex++) 
		{
			const BiomeInfo& biomeInfo = 
				BiomeInfo::biomeInfo[getBiome(chunkIndex)];

			xpos += biomeInfo.terrainFrequency * Chunk::width 
				* (negative ? -1 : 1);
		}

		return xpos;
	}
	int* generateHeightMap(int chunkOffset) {
		static int heightMap[Chunk::width]; 
		static FractalNoise heightGenerator; 

		const Biome::Id biome = getBiome(chunkOffset); 
		const BiomeInfo& biomeInfo = BiomeInfo::biomeInfo[biome]; 
		const float frequency = biomeInfo.terrainFrequency;
		const float startXposPosition = getChunkStartXpos(chunkOffset); 
		const float seedOffset = generatorSeed * seedScaler; 

		// Sets parameters based on the current biome. 
		heightGenerator.setBaseAmplitude(biomeInfo.terrainAmplitude);
		heightGenerator.setBaseFrequency(1.0f / 50.0f);
		heightGenerator.setOctaves(8);
		heightGenerator.setPersistence(0.5f);

		for (int chunkXpos = 0; chunkXpos < Chunk::width; chunkXpos++) {
			int& height = heightMap[chunkXpos];
			float xOffset = chunkXpos * frequency; 

			height = 64 + heightGenerator.noise(
				startXposPosition + xOffset, 0.0f, seedOffset
			); 
			gs::util::clamp(&height, 0, Chunk::height); 
		}

		return heightMap;
	}
	std::vector<gs::Vec2i>& generateCircleBlocks(gs::Vec2i position, float radius) {
		static std::vector<gs::Vec2i> blockPositions; 

		blockPositions.clear(); 
		blockPositions.push_back(position); 

		float innerRadius = 1.0f; 

		for (float angle = 0.0f; angle < 360.0f; angle += 
			360.0f / (2 * gs::util::PI * innerRadius * std::sqrt(2.0f)) * 1.1f) 
		{
			for (innerRadius = 1.0f; innerRadius < radius; 
				innerRadius += 1.0f)
			{
				const gs::Vec2i generatedPosition = position + gs::Vec2i(
					-0.5f + (innerRadius * std::cos(gs::util::toRadians(angle))),
					-0.5f + (innerRadius * std::sin(gs::util::toRadians(angle)))
				);

				bool duplicated = false; 

				for (auto& blockPosition : blockPositions) {
					if (blockPosition == generatedPosition) {
						duplicated = true;
						break; 
					}
				}

				if (!duplicated)
					blockPositions.push_back(generatedPosition); 
			}
		}

		return blockPositions; 
	}

	void generateTree(gs::Vec2i position, TreeType treeType, World& world) {
		auto placeLog = [&](gs::Vec2i position, Block::Id logId, 
			bool addLeaves = false) 
		{
			Block log = Block(logId); 

			log.tags.ignoreCollision = true; 

			if (addLeaves)
				log.tags.animationOffset = 2; 

			world.placeBlock(position, log); 
		}; 

		const int treeValueIndex = static_cast<int>(treeType); 
		const Block::Id logId = logTypes[treeValueIndex]; 
		const Block::Id leaveId = leaveTypes[treeValueIndex]; 
		const gs::Vec2i treeHeightRange = treeHeightRanges[treeValueIndex]; 
		const int topLeaves = treeTopLeaveCounts[treeValueIndex]; 
		const int treeHeight = treeHeightRange.x + (randomGenerator.generate() 
			% std::max(treeHeightRange.y - treeHeightRange.x + 1, 1)); 
		const int treeTop = std::max(0, position.y - treeHeight); 
		const bool blendLeaves = treeType != TreeType::Cactus; 

		int treeSeed = std::abs(position.x + (position.y * 3));
		Random randomGenerator(treeSeed);
		int acaciaLeaveDirection = 0; 

		// Generate tree logs. 
		for (int ypos = position.y; ypos > treeTop; ypos--) {
			bool addLeaves = false; 

			// Add leaves to top of tree if allowed. 
			if (blendLeaves) {
				if (ypos - treeTop <= topLeaves) 
					addLeaves = true; 
			}

			if (treeType == TreeType::Acacia) {
				if (randomGenerator.generate() % 6 == 0) {
					if (acaciaLeaveDirection == 0)
						acaciaLeaveDirection = randomGenerator.generate() % 2 
							== 0 ? -1 : 1;

					position.x += acaciaLeaveDirection; 
				}
			}

			placeLog({ position.x, ypos }, logId, addLeaves);
		}
		
		Block leave = Block(leaveId); 

		leave.tags.ignoreCollision = true; 

		switch (treeType) {
		case TreeType::Oak: case TreeType::Birch: case TreeType::Jungle:
			for (int ypos = 0; ypos < 4; ypos++) {
				const bool bottomLayer = ypos < 2; 
			
				for (int xpos = bottomLayer ? 1 : 0;
					xpos < (bottomLayer ? 4 : 5); xpos++)
				{
					const gs::Vec2i leavePosition = gs::Vec2i(
						position.x + xpos - 2, treeTop - 1 + ypos
					);
					
					// Places leaves around center, where the log is located. 
					if (leavePosition.x != position.x 
							|| leavePosition.y <= treeTop) 
						world.placeBlock(leavePosition, leave, 
							World::PlaceFilter::Fill);
				}
			} 

			break; 
		case TreeType::Spruce:
		{
			const int treeVariation = treeSeed % 2; 

			// Add top of spruce tree. 
			for (int peakIndex = 0; peakIndex < 3; peakIndex++) {
				const gs::Vec2i leavePosition = gs::Vec2i(
					position.x + peakIndex - 1,
					treeTop + ((1 + peakIndex) % 2)
				);

				world.placeBlock(leavePosition, leave,
					World::PlaceFilter::Fill);
			}

			const int leaveRingHeight = 2 + treeVariation; 

			for (int leaveRing = 1; leaveRing < (treeHeight / leaveRingHeight) 
				- 1; leaveRing++)
			{
				const int ringYpos = treeTop + (leaveRing * leaveRingHeight) + 1;

				for (int ypos = 0; ypos < (leaveRing < 2 ? 2 : 3); ypos++) {
					const int leaveRingWidth = ypos + 1;

					for (int xpos = -leaveRingWidth; xpos < leaveRingWidth + 1;
						xpos++) 
					{
						const gs::Vec2i leavePosition = gs::Vec2i(
							position.x - xpos,
							ringYpos + ypos
						);

						if (xpos == 0)
							placeLog(leavePosition, logId, true);
						else
							world.placeBlock(leavePosition, leave,
								World::PlaceFilter::Fill);
					}
				}
			}
		}
			break; 
		case TreeType::Acacia:
			const int treeVariation = treeSeed % 2; 
			const int treeTopThickness = 2 + treeVariation;
			const int treeRingStarts[3] = { 2, 0, 1 }; 
			const int treeRingEnds[3] = { 5, 7, 6 }; 

			for (int ypos = 0; ypos < treeTopThickness; ypos++) {
				const bool bottomLayer = ypos == 0;

				for (int xpos = treeRingStarts[ypos]; xpos < 
					treeRingEnds[ypos]; xpos++)
				{
					const gs::Vec2i leavePosition = gs::Vec2i(
						position.x + xpos - 3, treeTop - 1 + ypos 
							- treeVariation
					);

					// Places leaves around center, where the log is located. 
					if (leavePosition.x != position.x
							|| leavePosition.y <= treeTop)
						world.placeBlock(leavePosition, leave,
							World::PlaceFilter::Fill);
				}
			}

			break; 
		}
	}
	void generateTree(int xpos, int ypos, TreeType treeType, World& world) {
		generateTree({ xpos, ypos }, treeType, world); 
	}
	void generateFluidBody(PathFinder& pathFinder, Block::Id fluid, World& world) {
		const std::vector<gs::Vec2i>& searchedBlocks = 
			pathFinder.getSearchedBlocks(); 
		
		for (auto& fluidPosition : searchedBlocks) 
			world.setBlock(fluidPosition, fluid); 
	}
	void generateBiomeSpecificFeatures(
		Chunk& chunk, World& world, const int* heightMap) 
	{
		const int seed = chunk.offset + (generatorSeed * seedScaler);

		Random randomGenerator(seed); 
		gs::Vec2i blockPosition; 

		switch (chunk.getBiomeId()) {
		case Biome::Jungle:
			world.enableBlockUpdates();

			// Generates bamboo stalks. 
			for (blockPosition.x = 1; blockPosition.x < Chunk::width - 1;
				blockPosition.x++)
			{
				blockPosition.y = heightMap[blockPosition.x];

				if (!chunk.getBlock(blockPosition).isEmpty()) {
					// Bamboo stalk is between 2 and 15 blocks tall. 
					int remainingHeight = 2
						+ randomGenerator.generate() % 13;

					blockPosition.y--;

					while (chunk.getBlock(blockPosition).isEmpty()
						&& remainingHeight > 0)
					{
						world.placeBlock(
							blockPosition.x + (chunk.offset * Chunk::width),
							blockPosition.y, Block(Block::BambooStalk)
						);

						blockPosition.y--;
						remainingHeight--;
					}
				}
			}

			world.enableBlockUpdates(false);
			break; 
		case Biome::Swamp:
		{ 
			const int waterBodySpawnAttempts = 16; 
			const gs::Vec2i waterBodyVolumeRange = gs::Vec2i(1, 100); 

			world.enableBlockUpdates();

			Random waterBodyPositionGenerator(generatorSeed + chunk.offset);

			for (int waterBodySpawnAttempt = 0; waterBodySpawnAttempt <
				waterBodySpawnAttempts; waterBodySpawnAttempt++)
			{
				gs::Vec2i generatedPosition = gs::Vec2i(
					waterBodyPositionGenerator.generate() % Chunk::width, 0
				); 

				generatedPosition.y = heightMap[generatedPosition.x] 
					- (1 + (waterBodyPositionGenerator.generate() % 3)); 
				generatedPosition.x += chunk.offset * Chunk::width;

				world.addFluidBodyAttempt(
					generatedPosition, Block::Water, waterBodyVolumeRange
				); 
			}

			world.enableBlockUpdates(false);
		}
			break; 
		}
	}
	void generateChunk(Chunk& chunk, World& world) {
		static FractalNoise heightGenerator; 

		generatorSeed = world.seed; 
		world.enableBlockUpdates(false); 

		const Biome biome = chunk.getBiome(); 
		const BiomeInfo& biomeInfo = BiomeInfo::biomeInfo[biome.id]; 
		const Biome::Id prvsBiome = getBiome(chunk.offset - 1); 
		const int biomeSmoothingDistance = 8; 

		int* tempHeightMap = generateHeightMap(chunk.offset);
		int heightMap[Chunk::width]; 

		_memccpy(heightMap, tempHeightMap, Chunk::width, sizeof(int) 
			* Chunk::width); 

		if (prvsBiome != biome.id) {
			int correctionHeight = generateHeightMap(chunk.offset - 1)
				[Chunk::width - 1];

			for (int xpos = 0; xpos < biomeSmoothingDistance - 2; xpos++) {                                                              
				const float correctionDifference = heightMap[xpos] 
					- correctionHeight;
				const float scaler = (biomeSmoothingDistance - xpos - 1.0f)
					/ static_cast<float>(biomeSmoothingDistance);

				heightMap[xpos] -= correctionDifference * scaler;
			}
		}
		
		int defaultGeneratorIndex = 0,
			biomeGeneratorIndex = 0; 

		for (int generatorLayer = 0; generatorLayer < Generator::numOfLayers; 
			generatorLayer++) 
		{
			for (auto& generator : defaultGenerators) {
				if (generator.layer == generatorLayer) {
					applyGenerator(generator, chunk, world, heightMap, 
						defaultGeneratorIndex);
					defaultGeneratorIndex++; 
				}
			}
			for (auto& generator : biomeInfo.generators) {
				if (generator.layer == generatorLayer) {
					applyGenerator(generator, chunk, world, heightMap,
						biomeGeneratorIndex);
					biomeGeneratorIndex++; 
				}
			}
		}

		if (biomeInfo.treeType != TreeType::None) {
			Random treeValueGenerator(generatorSeed + chunk.offset); 
			int prvsTreeXpos = -1; 

			for (int xpos = 1; xpos < Chunk::width - 1; xpos++) {
				const int height = heightMap[xpos];
				const int distanceFromPrvsTree = xpos - prvsTreeXpos; 
				const int generatedValue = treeValueGenerator.generate() % 1000;

				if (distanceFromPrvsTree > 1 && generatedValue < 100.0f 
					* biomeInfo.treeFrequency) 
				{
					// Don't place tree on empty block. 
					if (chunk.getBlock(xpos, height).isEmpty())
						continue;

					generateTree(
						xpos + (chunk.offset * Chunk::width), height - 1,
						biomeInfo.treeType, world
					);

					prvsTreeXpos = xpos; 
				}
			}
		}
		
		auto calculateFluidBodyHeight = [](gs::Vec2i heightRange, 
			int generatedHeight, bool isWaterBody) -> int 
		{
			float normalizedHeight = static_cast<float>(generatedHeight)
				/ static_cast<float>(heightRange.y); 
			normalizedHeight = std::pow(normalizedHeight, isWaterBody ? 2.0f : 0.33f); 
			return heightRange.x + heightRange.y * normalizedHeight; 
		}; 

		Random fluidPositionGenerator(generatorSeed + chunk.offset); 

		for (int fluidBodySpawnAttempt = 0; fluidBodySpawnAttempt <
			waterBodySpawnAttempts + lavaBodySpawnAttempts; 
			fluidBodySpawnAttempt++)
		{
			gs::Vec2i generatedPosition;

			generatedPosition.x = fluidPositionGenerator.generate() 
				% Chunk::width;

			const bool isWaterBody = fluidBodySpawnAttempt < 
				waterBodySpawnAttempts; 
			const gs::Vec2i heightStretch = gs::Vec2i(
				heightMap[generatedPosition.x] + (isWaterBody 
					? waterBodyDistanceFromSurface : lavaBodyDistanceFromSurface),
				Chunk::height - (heightMap[generatedPosition.x] 
					+ (isWaterBody ? waterBodyDistanceFromSurface 
						: lavaBodyDistanceFromSurface))
			);

			generatedPosition.y = calculateFluidBodyHeight(
				heightStretch, fluidPositionGenerator.generate()
					% heightStretch.y, isWaterBody
			); 

			generatedPosition.x += chunk.offset * Chunk::width;

			world.addFluidBodyAttempt(
				generatedPosition, isWaterBody ? Block::Water : Block::Lava
			); 
		}

		world.enableBlockUpdates(); 

		for (int fluidFallSpawnAttempt = 0; fluidFallSpawnAttempt <
			waterFallSpawnAttempts + lavaFallSpawnAttempts; fluidFallSpawnAttempt++)
		{
			gs::Vec2i generatedPosition; 

			generatedPosition.x = 1 + (fluidPositionGenerator.generate() 
				% (Chunk::width - 2)); 

			const bool isWaterFall = fluidFallSpawnAttempt <
				waterFallSpawnAttempts; 
			const gs::Vec2i heightStretch = gs::Vec2i(
				heightMap[generatedPosition.x] + (isWaterFall
					? waterFallDistanceFromSurface : lavaFallDistanceFromSurface),
				Chunk::height - (heightMap[generatedPosition.x]
					+ (isWaterFall ? waterFallDistanceFromSurface
						: lavaBodyDistanceFromSurface))
			); 

			generatedPosition.y = calculateFluidBodyHeight(
				heightStretch, fluidPositionGenerator.generate()
					% heightStretch.y, isWaterFall
			); 

			generatedPosition.x += chunk.offset * Chunk::width;

			if (world.getBlock(generatedPosition).isSolid()
				&& (!world.getBlock(generatedPosition + gs::Vec2i(1, 0)).isSolid()
					|| !world.getBlock(generatedPosition + gs::Vec2i(-1, 0)).isSolid()
					|| !world.getBlock(generatedPosition + gs::Vec2i(0, 1)).isSolid()))
			{
				world.placeBlock(
					generatedPosition, isWaterFall ? Block::Water : Block::Lava
				); 
			}
		}

		world.enableBlockUpdates(false);

		generateBiomeSpecificFeatures(chunk, world, heightMap); 
		generateStructures(chunk, world, heightMap); 
		world.enableBlockUpdates(true);
	}
}