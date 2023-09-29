#include "../../hdr/world/World.hpp"
#include "../../hdr/world/Generation.hpp"
#include "../../hdr/graphics/UI.hpp"
#include "../../hdr/entity/Collision.hpp"
#include "../../hdr/audio/AudioEnviroment.hpp"

namespace engine {
	World::World() :
		versionNumber(mMajorVersion + mMinorVersion / 10.0f), 
		gameTime(10000),
		baseChunkOffset(0),
		blockUpdatesEnabled(true), 
		blocksUpdated(0)
	{
		generateSeed(); 
	}
	World::~World() {
	}

	void World::createWorld(const std::string& folderName, const std::string& worldName) {
		saveFileDirectory = "saves/" + folderName; 

		// Uses the folder's name if no world name is given. 
		name = worldName == "UseFolderName" ? folderName : worldName; 
		this->folderName = folderName; 

		// Creates the file directories to ensure data can be written to.  
		createWorldFileDirectories();
	}
	void World::loadWorld(const std::string& folderName) {
		saveFileDirectory = "saves/" + folderName;
		 
		this->folderName = folderName; 

		loadWorldProperties();
		loadPlayer(); 
		loadTileList(); 
		loadFluidBodyAttemptList(); 

		// Creates the file directories to ensure data can be written to.  
		createWorldFileDirectories(); 
	}
	void World::saveWorld() {
		createWorldFileDirectories();

		saveWorldProperties(); 

		// Saves all currently loaded chunks. 
		for (auto& chunk : chunks) {
			if (chunk->needsToBeSaved)
				saveChunk(*chunk);

			saveChunkEntities(*chunk); 
		}

		savePlayer(); 
		// Save unplaced tiles. 
		saveTileList(); 
		saveFluidBodyAttemptList(); 
	}
	void World::saveIcon() {
		// Only saves icon if icon doesn't exist yet. 
		if (!std::filesystem::exists(getIconSaveFileName())) {
			const sf::Image& screenshot = render::takeScreenshot();

			sf::Texture screenshotTexture;
			sf::Sprite screenshotSprite;
			sf::RenderTexture iconTexture;

			// Copy screenshot data to a textue. 

			screenshotTexture.create(screenshot.getSize().x, screenshot.getSize().y);
			screenshotTexture.update(screenshot);
			screenshotSprite.setTexture(screenshotTexture, true);
			screenshotSprite.setOrigin(
				screenshot.getSize().x / 2.0f, screenshot.getSize().y / 2.0f
			);
			screenshotSprite.setScale(0.2f, 0.2f);

			// Handle the texture of the new icon. 

			iconTexture.create(
				render::ui::worldPreviewIconSize.x,
				render::ui::worldPreviewIconSize.y
			);
			screenshotSprite.setPosition(
				iconTexture.getSize().x / 2.0f, iconTexture.getSize().y / 2.0f
			);
			iconTexture.draw(screenshotSprite);
			iconTexture.display();

			// Change icon texture into an actual image so that it can be saved. 

			const sf::Image& icon = iconTexture.getTexture().copyToImage();

			icon.saveToFile(getIconSaveFileName());
		}
	}

	void World::generateSeed() {
		static Random randomSeedGenerator;
		seed = randomSeedGenerator.generate() % INT16_MAX; 
	}
	void World::update() {
		// Just an alias for the normalized camera position. 
		const gs::Vec2f& cameraPosition = render::normalizedCameraPosition; 

		// Unloading chunks
		for (int chunkIndex = 0; chunkIndex < chunks.size(); chunkIndex++) {
			const Chunk* chunk = chunks[chunkIndex].get(); 

			int chunkXpos = (chunk->offset * Chunk::width) + (Chunk::width / 2);
			int distanceFromCamera = std::abs(cameraPosition.x - chunkXpos);

			// Removes chunk if it passes the unloading distance. 
			if (distanceFromCamera > chunkUnloadDistance * Chunk::width) {
				if (chunk->needsToBeSaved)
					saveChunk(*chunk); 

				saveChunkEntities(*chunk); 

				chunks.erase(chunks.begin() + chunkIndex); 
				chunkIndex--; 
			}
		}

		// Loading chunks

		const int cameraChunkOffset = cameraPosition.x / Chunk::width; 

		for (int chunkOffset = cameraChunkOffset - chunkLoadDistance;
			chunkOffset < cameraChunkOffset + chunkLoadDistance; chunkOffset++)
		{
			// Adds chunk if it is missing from the chunks vector. 
			if (getChunkSafely(chunkOffset) == nullptr) {
				generatorSeed = seed; 

				Chunk newChunk(chunkOffset, getBiome(chunkOffset)); 

				int insertionIndex = 0; 

				// Finds insertion point that keeps the chunks sorted. 
				for (auto& chunk : chunks) {
					if (chunk.get()->offset < chunkOffset)
						insertionIndex++; 
				}

				chunks.insert(
					chunks.begin() + insertionIndex, 
					std::make_unique<Chunk>(newChunk)
				); 
				baseChunkOffset = chunks[0]->offset;

				Chunk* addedChunk = getChunk(chunkOffset); 

				// If chunk is unable to be loaded, the chunk gets generated. 
				if (!loadChunk(*addedChunk)) {
					generateChunk(*addedChunk, *this);
					addedChunk->needsToBeSaved = true;
				}
				else
					addedChunk->needsToBeSaved = false; 

				loadChunkEntities(*addedChunk); 
			}
		}

		// Adding unplaced tiles
		if (!chunks.empty()) {
			// Updates the chunk offset so that the first chunk is the start. 
			baseChunkOffset = chunks[0]->offset;

			for (int tileIndex = 0; tileIndex < tileList.size(); 
				tileIndex++) 
			{
				const TilePlacement& tilePlacement = tileList[tileIndex];

				bool removeTile = false;

				switch (tilePlacement.placeFilter) {
				case PlaceFilter::Replace:
					// Simply replaces the tile in place. 
					if (tilePlacement.useBlock) {
						removeTile = setBlock(tilePlacement.position, tilePlacement.block);
						triggerBlockUpdates(tilePlacement.position); 
					}
					else
						removeTile = setWall(tilePlacement.position, tilePlacement.wall);

					break; 
				case PlaceFilter::Fill:
					if (getChunk(getChunkOffset(tilePlacement.position.x)) != nullptr) {
						if (tilePlacement.useBlock) {
							const Block currentBlock = getBlock(tilePlacement.position);

							if (currentBlock.isEmpty()
								|| currentBlock.getVar(BlockInfo::generationReplacable))
							{
								setBlock(tilePlacement.position, tilePlacement.block);
								triggerBlockUpdates(tilePlacement.position);
							}
						}
						else {
							const Wall currentWall = getWall(tilePlacement.position); 

							if (currentWall.isEmpty()) 
								setWall(tilePlacement.position, tilePlacement.wall);
						}

						removeTile = true; 
					}

					break; 
				}

				// Removes the block from the list once it has been placed. 
				if (removeTile) {
					tileList.erase(tileList.begin() + tileIndex);
					tileIndex--;
				}
			}

			for (int fluidBodyAttemptIndex = 0; fluidBodyAttemptIndex <
				fluidBodyAttemptList.size(); fluidBodyAttemptIndex++)
			{
				const FluidBodyAttempt& fluidBodyAttempt = 
					fluidBodyAttemptList[fluidBodyAttemptIndex]; 

				bool removeFluidBody = false; 

				if (getChunk(getChunkOffset(fluidBodyAttempt.position.x
						+ maxFluidBodyStretch)) != nullptr
					&& getChunk(getChunkOffset(fluidBodyAttempt.position.x
						- maxFluidBodyStretch)) != nullptr)
				{
					//const bool isWaterBody = fluidBodyAttempt.fluid.id 
					//	== Block::Water;

					PathFinder pathFinder; 

					pathFinder.startPoint = fluidBodyAttempt.position; 
					pathFinder.restrictions.top = pathFinder.startPoint.y;	
					pathFinder.setSafeBlockFilter([](engine::Block block) -> bool { 
						return block.isFluidBreakable(); 
					});
					pathFinder.setUnsafeBlockFilter([](Block block) -> bool {
						for (int treeIndex = 1; treeIndex <
							static_cast<int>(TreeType::End); treeIndex++)
						{
							const Block::Id logId = logTypes[treeIndex];
							const Block::Id leaveId = leaveTypes[treeIndex];
					
							if ((logId != Block::Air && block.id == logId)
								|| (leaveId != Block::Air && block.id == leaveId))
							{
								return true;
							}
						}
					
						return block.isFluid();
					});

					const int calculatedVolume = pathFinder.calculateAreaVolume(
						fluidBodyAttempt.fluidSizeRange.y
					);

					if (!pathFinder.wereChunkBoundsReached()
						&& calculatedVolume < fluidBodyAttempt.fluidSizeRange.y
						&& calculatedVolume > fluidBodyAttempt.fluidSizeRange.x)
					{
						generateFluidBody(
							pathFinder, fluidBodyAttempt.fluidId, *this
						); 
					}

					removeFluidBody = true; 
				}

				if (removeFluidBody) {
					fluidBodyAttemptList.erase(
						fluidBodyAttemptList.begin() + fluidBodyAttemptIndex
					);
					fluidBodyAttemptIndex--; 
				}
			}
		}

		updateBlocks(); 

		gameTime.tick(1); 
	}
	void World::enableBlockUpdates(bool enable) {
		blockUpdatesEnabled = enable; 
	}

	bool World::isValidBlockPlacementLocation(gs::Vec2i position, Block::Id blockId) {
		bool isValid = (!getBlock(position.x, position.y - 1).isEmpty()
			|| !getBlock(position.x + 1, position.y).isEmpty()
			|| !getBlock(position.x, position.y + 1).isEmpty()
			|| !getBlock(position.x - 1, position.y).isEmpty()
			|| !getWall(position).isEmpty())
			&& (getBlock(position).isEmpty() || getBlock(position).isFluid());
		isValid = isValid && isValidBlock(position, blockId); 
		return isValid;
	}
	bool World::isValidBlockPlacementLocation(int xpos, int ypos, Block::Id blockId) {
		return isValidBlockPlacementLocation({ xpos, ypos }, blockId); 
	}
	bool World::isValidWallPlacementLocation(gs::Vec2i position, Wall::Id wallId) {
		return getWall(position).isEmpty();
	}
	bool World::isValidWallPlacementLocation(int xpos, int ypos, Wall::Id wallId) {
		return isValidWallPlacementLocation({ xpos, ypos }, wallId); 
	}
	void World::placeBlock(
		gs::Vec2i position, Block block, PlaceFilter placeFilter, 
		bool playSoundEvent)
	{
		if (isValidYpos(position.y)) [[likely]] { 
			bool addBlockToList = false; 
			bool soundEventAvailable = true; 

			switch (placeFilter) {
			case PlaceFilter::Replace:
				addBlockToList = !setBlock(position, block); 
				triggerBlockUpdates(position); 
				break; 
			case PlaceFilter::Fill:
				if (getChunk(getChunkOffset(position.x)) != nullptr) {
					const Block currentBlock = getBlock(position); 

					if (currentBlock.isEmpty()
						|| currentBlock.getVar(BlockInfo::generationReplacable))
					{
						setBlock(position, block);
						triggerBlockUpdates(position);
					}
					else
						soundEventAvailable = false; 

					return; 
				}
				else 
					addBlockToList = true; 

				break; 
			}

			if (addBlockToList) {
				TilePlacement blockPlacement; 

				blockPlacement.position = position; 
				blockPlacement.block = block; 
				blockPlacement.placeFilter = placeFilter; 
				blockPlacement.useBlock = true; 

				tileList.push_back(blockPlacement);
			}
			else if (playSoundEvent && soundEventAvailable) {
				const audio::SoundEvent::Id blockSoundEvent = 
					static_cast<audio::SoundEvent::Id>(block.getVar(
						BlockInfo::soundEvent)); 

				if (blockSoundEvent != audio::SoundEvent::None) {
					const float distanceToPlayer = gs::util::distance(
						gs::Vec2f(position), player->position
					); 

					audio::SoundEvent::soundEvents[blockSoundEvent]
						.playSoundEvent(audio::SoundEvent::EventType::TilePlacement, 
							distanceToPlayer); 
				}
			}
		}
	}
	void World::placeBlock(
		int xpos, int ypos, Block block, PlaceFilter placeFilter,
		bool playSoundEvent)
	{
		placeBlock({ xpos, ypos }, block, placeFilter, playSoundEvent); 
	}
	void World::placeBlock(
		gs::Vec2i position, Block::Id blockId, PlaceFilter placeFilter, 
		bool playSoundEvent)
	{
		placeBlock(position, Block(blockId), placeFilter, playSoundEvent); 
	}
	void World::placeBlock(
		int xpos, int ypos, Block::Id blockId, PlaceFilter placeFilter, 
		bool playSoundEvent)
	{
		placeBlock({ xpos, ypos }, blockId, placeFilter, playSoundEvent); 
	}
	void World::placeWall(
		gs::Vec2i position, Wall wall, PlaceFilter placeFilter,
		bool playSoundEvent) 
	{
		if (isValidYpos(position.y)) [[likely]] {
			bool addWallToList = false;
			bool soundEventAvailable = true;

			switch (placeFilter) {
			case PlaceFilter::Replace:
				addWallToList = !setWall(position, wall);
				break;
			case PlaceFilter::Fill:
				if (getChunk(getChunkOffset(position.x)) != nullptr) {
					if (getWall(position).isEmpty())
						setWall(position, wall);
					else
						return; 
				}
				else
					addWallToList = true;

				break;
			}

			if (addWallToList) {
				TilePlacement wallPlacement; 

				wallPlacement.position = position;
				wallPlacement.wall = wall; 
				wallPlacement.placeFilter = placeFilter;
				wallPlacement.useBlock = false;

				tileList.push_back(wallPlacement);
			}
			else if (playSoundEvent && soundEventAvailable) {
				const audio::SoundEvent::Id wallSoundEvent =
					static_cast<audio::SoundEvent::Id>(wall.getVar(
						WallInfo::soundEvent));

				if (wallSoundEvent != audio::SoundEvent::None) {
					const float distanceToPlayer = gs::util::distance(
						gs::Vec2f(position), player->position
					);

					audio::SoundEvent::soundEvents[wallSoundEvent]
						.playSoundEvent(audio::SoundEvent::EventType::TilePlacement,
							distanceToPlayer);
				}
			}
		}
	}
	void World::placeWall(
		int xpos, int ypos, Wall wall, PlaceFilter placeFilter, 
		bool playSoundEvent) 
	{
		placeWall({ xpos, ypos }, wall, placeFilter, playSoundEvent);
	}
	void World::placeWall(
		gs::Vec2i position, Wall::Id wallId, PlaceFilter placeFilter, 
		bool playSoundEvent)
	{
		placeWall(position, Wall(wallId), placeFilter, playSoundEvent); 
	}
	void World::placeWall(
		int xpos, int ypos, Wall::Id wallId, PlaceFilter placeFilter, 
		bool playSoundEvent)
	{
		placeWall({ xpos, ypos }, wallId, placeFilter, playSoundEvent); 
	}
	void World::breakBlock(
		gs::Vec2i position, bool dropItem, bool playSoundEvent, 
		bool generateParticles)
	{
		const Block block = getBlock(position); 
		const Item::Id itemId = static_cast<Item::Id>(
			block.getVar(BlockInfo::itemDrop)
		); 

		// Drops item on ground if required. 
		if (dropItem) {
			const Item item = Item(itemId); 
			const gs::Vec2f itemPosition = 
				gs::Vec2f(position.x + 0.5f, position.y + 0.5f); 

			if (!item.isEmpty()) {
				ItemEntity::dropItemEntity(
					ItemContainer(Item(itemId), 1), itemPosition
				);
			}

			if (item.getVar(BlockInfo::requiresTileEntity)) 
				removeTileEntity(position); 

			const LootTable::Id lootTableId =
				static_cast<LootTable::Id>(block.getVar(BlockInfo::lootTable)); 

			if (lootTableId != LootTable::None) {
				ItemContainer generatedItemContainer;

				do {
					generatedItemContainer =
						LootTable::lootTables[lootTableId].getLoot();

					ItemEntity::dropItemEntity(
						generatedItemContainer, itemPosition
					);
				} 
				while (!generatedItemContainer.item.isEmpty());
			}
		}

		if (playSoundEvent) {
			const audio::SoundEvent::Id blockSoundEvent =
				static_cast<audio::SoundEvent::Id>(block.getVar(
					BlockInfo::soundEvent));

			if (blockSoundEvent != audio::SoundEvent::None) {
				const float distanceToPlayer = gs::util::distance(
					gs::Vec2f(position), player->position
				);

				audio::SoundEvent::soundEvents[blockSoundEvent]
					.playSoundEvent(audio::SoundEvent::EventType::TileBroken,
						distanceToPlayer);
			}
		}

		if (generateParticles && render::shouldParticlesBeRendered)
			Particle::generateBlockParticles(position, true); 

		setBlock(position, Block::Air); 
		triggerBlockUpdates(position); 
	}
	void World::breakBlock(
		int xpos, int ypos, bool dropItem, bool playSoundEvent, 
		bool generateParticles)
	{
		breakBlock({ xpos, ypos }, dropItem, playSoundEvent, generateParticles); 
	}
	void World::breakWall(
		gs::Vec2i position, bool dropItem, bool playSoundEvent, 
		bool generateParticles)
	{
		const Wall wall = getWall(position); 
		const Item::Id itemId = static_cast<Item::Id>(
			getWall(position).getVar(WallInfo::itemDrop)
		);

		// Drops item on ground if required. 
		if (dropItem) {
			const Item item = Item(itemId);

			if (!item.isEmpty()) {
				ItemEntity::dropItemEntity(
					ItemContainer(Item(itemId), 1),
					gs::Vec2f(position.x + 0.5f, position.y + 0.5f)
				);
			}
		}

		if (playSoundEvent) {
			const audio::SoundEvent::Id wallSoundEvent =
				static_cast<audio::SoundEvent::Id>(wall.getVar(
					WallInfo::soundEvent));

			if (wallSoundEvent != audio::SoundEvent::None) {
				const float distanceToPlayer = gs::util::distance(
					gs::Vec2f(position), player->position
				);

				audio::SoundEvent::soundEvents[wallSoundEvent]
					.playSoundEvent(audio::SoundEvent::EventType::TileBroken,
						distanceToPlayer);
			}
		}

		if (generateParticles && render::shouldParticlesBeRendered)
			Particle::generateWallParticles(position, true);

		setWall(position, Wall::Air);
	}
	void World::breakWall(
		int xpos, int ypos, bool dropItem, bool playSoundEvent, 
		bool generateParticles)
	{
		breakWall({ xpos, ypos }, dropItem, playSoundEvent, generateParticles);
	}
	void World::addTileEntity(TileEntity tileEntity) {
		Chunk* chunk = getChunk(getChunkOffset(tileEntity.position.x));

		tileEntity.position = getChunkPosition(tileEntity.position);

		if (chunk != nullptr && isValidYpos(tileEntity.position.y)) [[likely]]
			chunk->addTileEntity(tileEntity);
	}
	void World::createTileEntity(gs::Vec2i position) {
		Chunk* chunk = getChunk(getChunkOffset(position.x));

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			chunk->createTileEntity(getChunkPosition(position));
	}
	void World::createTileEntity(int xpos, int ypos) {
		createTileEntity({ xpos, ypos });
	}
	void World::removeTileEntity(gs::Vec2i position) {
		TileEntity* tileEntity = getTileEntity(position); 

		if (tileEntity != nullptr) {
			// Iterates through all of the available item containers, and drops
			// them on the ground. 
			for (int itemContainerIndex = 0; itemContainerIndex <
				TileEntity::getItemContainerSize(getBlock(position)); 
				itemContainerIndex++)
			{
				ItemEntity::dropItemEntity(
					tileEntity->getItemContainer(itemContainerIndex), 
					gs::Vec2f(position.x, position.y) + gs::Vec2f(0.5f, 0.5f)
				); 
			}

			// Removes tile entity data from the chunk which it resides. 
			getChunk(getChunkOffset(position.x))->removeTileEntity(
				getChunkPosition(position)
			);
		}
	}
	void World::removeTileEntity(int xpos, int ypos) {
		removeTileEntity({ xpos, ypos }); 
	}
	void World::addFluidBodyAttempt(
		gs::Vec2i position, Block::Id fluidId, gs::Vec2i sizeRange) 
	{
		FluidBodyAttempt fluidBodyAttempt; 

		fluidBodyAttempt.position = position; 
		fluidBodyAttempt.fluidId = fluidId; 
		
		if (sizeRange == gs::Vec2i(-1, -1)) {
			sizeRange = fluidId == Block::Water ? waterBodyVolumeRange
				: lavaBodyVolumeRange; 
		}

		fluidBodyAttempt.fluidSizeRange = sizeRange; 

		fluidBodyAttemptList.push_back(fluidBodyAttempt); 
	}

	bool World::setBlock(gs::Vec2i position, Block block) {
		Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]] {
			chunk->setBlock(getChunkPosition(position), block); 
			return true; 
		}

		return false;
	}
	bool World::setBlock(int xpos, int ypos, Block block) {
		return setBlock({ xpos, ypos }, block); 
	}
	bool World::setBlockId(gs::Vec2i position, Block::Id blockId) {
		Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]] {
			chunk->setBlockId(getChunkPosition(position), blockId); 
			return true; 
		}

		return false; 
	}
	bool World::setBlockId(int xpos, int ypos, Block::Id blockId) {
		return setBlockId({ xpos, ypos }, blockId); 
	}
	bool World::setWall(gs::Vec2i position, Wall wall) {
		Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]] {
			chunk->setWall(getChunkPosition(position), wall); 
			return true; 
		}

		return false;
	}
	bool World::setWall(int xpos, int ypos, Wall wall) {
		return setWall({ xpos, ypos }, wall); 
	}
	bool World::setWallId(gs::Vec2i position, Wall::Id wallId) {
		Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]] {
			chunk->setWallId(getChunkPosition(position), wallId); 
			return true; 
		}

		return false;
	}
	bool World::setWallId(int xpos, int ypos, Wall::Id wallId) {
		return setWallId({ xpos, ypos }, wallId);
	}
	void World::setTileColor(gs::Vec2i position, TileColor tileColor) {
		Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			chunk->setTileColor(getChunkPosition(position), tileColor);
	}
	void World::setTileColor(int xpos, int ypos, TileColor tileColor) {
		setTileColor({ xpos, ypos }, tileColor); 
	}

	Block World::getBlock(gs::Vec2i position) const {
		const Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getBlock(getChunkPosition(position)); 

		return Block(); 
	}
	Block World::getBlock(int xpos, int ypos) const {
		return getBlock({ xpos, ypos }); 
	}
	Block& World::getBlockRef(gs::Vec2i position) {
		static Block emptyBlock = Block(); 

		Chunk* chunk = getChunk(getChunkOffset(position.x));

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getBlockRef(getChunkPosition(position));

		return emptyBlock;
	}
	Block& World::getBlockRef(int xpos, int ypos) {
		return getBlockRef({ xpos, ypos }); 
	}
	Block::Id World::getBlockId(gs::Vec2i position) const {
		const Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getBlockId(getChunkPosition(position));

		return Block::Air; 
	}
	Block::Id World::getBlockId(int xpos, int ypos) const {
		return getBlockId({ xpos, ypos }); 
	}
	Wall World::getWall(gs::Vec2i position) const {
		const Chunk* chunk = getChunk(getChunkOffset(position.x)); 
		
		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getWall(getChunkPosition(position)); 

		return Wall(); 
	}
	Wall World::getWall(int xpos, int ypos) const {
		return getWall({ xpos, ypos });
	}
	Wall& World::getWallRef(gs::Vec2i position) {
		static Wall emptyWall = Wall();

		Chunk* chunk = getChunk(getChunkOffset(position.x));

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getWallRef(getChunkPosition(position));

		return emptyWall;
	}
	Wall& World::getWallRef(int xpos, int ypos) {
		return getWallRef({ xpos, ypos }); 
	}
	Wall::Id World::getWallId(gs::Vec2i position) const {
		const Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getWallId(getChunkPosition(position)); 

		return Wall::Air; 
	}
	Wall::Id World::getWallId(int xpos, int ypos) const {
		return getWallId({ xpos, ypos }); 
	}
	TileEntity* World::getTileEntity(gs::Vec2i position) {
		Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getTileEntity(getChunkPosition(position)); 

		return nullptr;
	}
	TileEntity* World::getTileEntity(int xpos, int ypos) {
		return getTileEntity({ xpos, ypos });
	}
	TileColor World::getTileColor(gs::Vec2i position) const {
		const Chunk* chunk = getChunk(getChunkOffset(position.x)); 

		if (chunk != nullptr && isValidYpos(position.y)) [[likely]]
			return chunk->getTileColor(getChunkPosition(position)); 

		return TileColor::Black; 
	}
	TileColor World::getTileColor(int xpos, int ypos) const {
		return getTileColor({ xpos, ypos }); 
	}
	Chunk* World::getChunk(int chunkOffset) const {
		// Calculates the index of the requested chunk based on how far away it
		// is from the first chunk. 
		const int chunkIndex = chunkOffset - baseChunkOffset;

		if (chunkIndex >= 0 && chunkIndex < chunks.size())
			return chunks[chunkIndex].get();

		return nullptr;
	}
	int World::getNumOfBlocksUpdated() const {
		return blocksUpdated;
	}
	bool World::isBlockExposedToSky(gs::Vec2i position) const {
		gs::Vec2i blockPosition = gs::Vec2i(position);
		bool skyReached = true;

		while (blockPosition.y > 0) {
			if (world->getBlock(blockPosition).isSolid()) {
				skyReached = false;
				break;
			}

			blockPosition.y--;
		}

		return skyReached;
	}
	bool World::isBlockExposedToSky(int xpos, int ypos) const {
		return isBlockExposedToSky({ xpos, ypos }); 
	}

	const gs::Vec2i World::blockUpdateRange = gs::Vec2i(60, 30);

	int World::getChunkOffset(int xpos) {
		return xpos >= 0 ? xpos / Chunk::width
			// Once the xpos is negative it must start at an offset of -1,
			// hence the minus 1. 
			: ((xpos + 1) / Chunk::width) - 1; 
	}

	World::TilePlacement::TilePlacement() :
		placeFilter(PlaceFilter::Fill),
		useBlock(true)
	{
	}
	World::FluidBodyAttempt::FluidBodyAttempt() {
	}

	void World::createWorldFileDirectories() const {
		const std::filesystem::path pathname = saveFileDirectory + "/chunks/";
		std::filesystem::create_directories(pathname);
	}

	void World::loadWorldProperties() {
		const PairVector& pairs = loadPairedFile(getSaveFileName());

		// Reads properties from save file. 
		for (auto& [attribute, value] : pairs) {
			if (attribute == "name")
				name = value;
			else if (attribute == "versionNumber")
				versionNumber = std::stof(value); 
			else if (attribute == "seed")
				seed = std::stoi(value);
			else if (attribute == "gameTime")
				gameTime.gameTicks = std::stoi(value);
		}
	}
	bool World::loadChunk(Chunk& chunk) {
		enum ReadState { Properties, BlockData, WallData };

		const std::filesystem::path pathname = 
			getChunkSaveFileName(chunk.offset);

		// Stops loading process if the chunk file doesn't exist. 
		if (!std::filesystem::exists(pathname))
			return false; 

		std::ifstream ifile(pathname); 
		std::string line; 

		ReadState readState = Properties; 
		gs::Vec2i chunkPosition; 
		std::string number;

		while (std::getline(ifile, line)) {
			switch (readState) {
			case Properties:
			{
				if (line == "BlockData") {
					readState = BlockData;
					break;
				}

				const PairVector& pairs = loadPairedFile(
					pathname.generic_string(), '=', "BlockData"
				);

				// Read properties from the chunk save file. 
				for (auto& [attribute, value] : pairs) {
					if (attribute == "biome")
						chunk.setBiomeId(static_cast<Biome::Id>(std::stoi(value)));
				}
			}
				break; 
			case BlockData: 
			{
				if (line == "WallData") {
					readState = WallData;
					chunkPosition.y = 0; 
					break; 
				}

				Block block;

				chunkPosition.x = 0;

				for (char symbol : line) {
					if (std::isdigit(symbol)) 
						number += symbol;
					else if (symbol == ':') {
						block.id = static_cast<Block::Id>(std::stoi(number));
						number = "";
					}
					else if (symbol == ' ') {
						block.tags.asInt = std::stoull(number);
						number = "";

						chunk.setBlock(chunkPosition, block);
						block = Block();

						chunkPosition.x++;
					}
				}

				chunkPosition.y++;
			}
				break; 
			case WallData: 
			{
				Wall wall; 

				chunkPosition.x = 0;

				for (char symbol : line) {
					if (std::isdigit(symbol))
						number += symbol;
					else if (symbol == ':') {
						wall.id = static_cast<Wall::Id>(std::stoi(number));
						number = "";
					}
					else if (symbol == ' ') {
						wall.tags.asInt = std::stoull(number);
						number = "";

						chunk.setWall(chunkPosition, wall);
						wall = Wall(); 

						chunkPosition.x++;
					}
				}

				chunkPosition.y++;
			}
				break; 
			}
		}

		return true;
	}
	bool World::loadChunkEntities(Chunk& chunk) {
		enum ReadState { TileEntity_, Entity_ };

		const std::filesystem::path pathname = 
			getChunkEntitySaveFileName(chunk.offset);

		// Stops loading process if the chunk entity file doesn't exist. 
		if (!std::filesystem::exists(pathname))
			return false;

		const PairVector& pairs = loadPairedFile(pathname.generic_string());

		ReadState readState = TileEntity_;
		gs::Vec2i chunkPosition;
		std::string number;
		
		TileEntity tileEntity; 
		bool createNewTileEntity = false;
		Entity* entity = nullptr; 
		Mob* mobEntity = nullptr; 
		ItemEntity* itemEntity = nullptr; 
		Entity::Type entityType = Entity::Type::None; 

		for (auto& [attribute, value] : pairs) {
			switch (readState) {
			case TileEntity_:
			{
				if (attribute == "Entity") {
					readState = Entity_;
					break;
				}
				else {
					if (attribute == "NewTileEntity") {
						if (createNewTileEntity) {
							addTileEntity(tileEntity);
							tileEntity = TileEntity();
							createNewTileEntity = false;
						}
					}
					else if (attribute == "xpos") {
						tileEntity.position.x = std::stoi(value);
						tileEntity.position.x += chunk.offset * Chunk::width;
					}
					else if (attribute == "ypos")
						tileEntity.position.y = std::stoi(value);
					else if (attribute == "tags")
						tileEntity.tags.asInt = std::stoull(value); 
					else if (attribute == "items") {				
						std::string number; 
						int containerIndex = 0; 
						int commaIndex = 0; 

						for (char chr : value) {
							if (chr == ' ') {
								tileEntity.setItemCount(containerIndex, 
									std::stoi(number)); 
	
								number = ""; 
								containerIndex++;
								commaIndex = 0;
							}
							else if (chr == ':') {
								switch (commaIndex) {
								case 0:
									tileEntity.setItemId(
										containerIndex,
										static_cast<Item::Id>(std::stoi(number))
									);
									break;
								case 1:
									tileEntity.setItemTags(
										containerIndex, std::stoull(number)
									);
									break;
								}

								number = ""; 
								commaIndex++;
							}
							else 
								number += chr; 
						}

						createNewTileEntity = true; 
					}
				}
			}
				break;
			case Entity_:
			{
				if (attribute == "NewEntity") {
					if (entityType != Entity::Type::None) {
						addEntity(entityType, entity);
						entity = nullptr; 
					}
				}
				else if (attribute == "type") {
					entityType = static_cast<Entity::Type>(std::stoi(value));

					switch (entityType) {
					case Entity::Type::Mob:
						mobEntity = new Mob(); 
						entity = mobEntity; 
						break; 
					case Entity::Type::ItemEntity:
						itemEntity = new ItemEntity();
						entity = itemEntity;
						break;
					}
				}
				else if (entity != nullptr) {
					if (attribute == "xpos")
						entity->position.x = std::stof(value);
					else if (attribute == "ypos")
						entity->position.y = std::stof(value);
					else if (attribute == "width")
						entity->size.x = std::stof(value);
					else if (attribute == "height")
						entity->size.y = std::stof(value);
					else if (attribute == "mass")
						entity->mass = std::stof(value);
					else {
						switch (entityType) {
						case Entity::Type::Mob:
							if (attribute == "mobType")
								mobEntity->type = 
									static_cast<Mob::Type>(std::stoi(value)); 
							break;
						case Entity::Type::ItemEntity:
							if (attribute == "itemId")
								itemEntity->itemContainer.item.id =
									static_cast<Item::Id>(std::stoi(value));
							else if (attribute == "itemTags")
								itemEntity->itemContainer.item.tags.asInt =
									std::stoull(value);
							else if (attribute == "itemCount")
								itemEntity->itemContainer.count =
									std::stoi(value);
							else if (attribute == "timeLeft")
								itemEntity->timeLeft = std::stoi(value);
							break;
						}
					}
				}
			}
				break;
			}
		}

		// Adds un-added entities. 
		if (createNewTileEntity) 
			addTileEntity(tileEntity); 
		if (entity != nullptr) 
			addEntity(entityType, entity); 

		return true;
	}
	bool World::loadPlayer() {
		const std::filesystem::path pathname = getPlayerSaveFileName();

		// Stops loading process if the chunk file doesn't exist. 
		if (!std::filesystem::exists(pathname))
			return false;

		const PairVector& pairs = loadPairedFile(pathname.generic_string());

		// If this file is able to be loaded, then it means the player was
		// already placed on the ground when the world was created. 
		player->placeOnGround = false; 

		for (auto& [attribute, value] : pairs) {
			if (attribute == "xpos")
				player->position.x = std::stof(value);
			else if (attribute == "ypos")
				player->position.y = std::stof(value);
			else if (attribute == "width")
				player->size.x = std::stof(value);
			else if (attribute == "height")
				player->size.y = std::stof(value);
			else if (attribute == "mass")
				player->mass = std::stof(value);
			else if (attribute == "health")
				player->health = std::stoi(value);
			else if (attribute == "foodLevel")
				player->foodLevel = std::stoi(value); 
			else if (attribute == "items") {
				std::string number;
				int containerIndex = 0;
				int commaIndex = 0;

				for (char chr : value) {
					if (chr == ' ') {
						player->inventory[containerIndex].count = 
							std::stoi(number);

						number = "";
						containerIndex++;
						commaIndex = 0;
					}
					else if (chr == ':') {
						switch (commaIndex) {
						case 0:
							player->inventory[containerIndex].item.id =
								static_cast<Item::Id>(std::stoi(number));
							break;
						case 1: 
							player->inventory[containerIndex].item.tags.asInt = 
								std::stoull(number);
							break;
						}

						number = "";
						commaIndex++;
					}
					else
						number += chr;
				}
			}
		}

		return true;
	}
	bool World::loadTileList() {
		const std::filesystem::path pathname = getTileListSaveFileName();

		// Stops loading process if the tile list file doesn't exist. 
		if (!std::filesystem::exists(pathname))
			return false;

		const PairVector& pairs = loadPairedFile(pathname.string());

		TilePlacement tilePlacement;

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewTile") {
				tileList.push_back(tilePlacement);
				tilePlacement = TilePlacement();
			}
			else if (attribute == "xpos")
				tilePlacement.position.x = std::stoi(value);
			else if (attribute == "ypos")
				tilePlacement.position.y = std::stoi(value);
			else if (attribute == "blockId")
				tilePlacement.block.id = static_cast<Block::Id>(
					std::stoi(value)
				);
			else if (attribute == "blockTags")
				tilePlacement.block.tags.asInt = std::stoull(value);
			else if (attribute == "wallId")
				tilePlacement.wall.id = static_cast<Wall::Id>(std::stoi(value));
			else if (attribute == "wallTags")
				tilePlacement.wall.tags.asInt = std::stoull(value);
			else if (attribute == "placeFilter")
				tilePlacement.placeFilter = static_cast<PlaceFilter>(
					std::stoi(value)
				);
			else if (attribute == "useBlock")
				tilePlacement.useBlock = std::stoi(value);
		}

		return true;
	}
	bool World::loadFluidBodyAttemptList() {
		const std::filesystem::path pathname = 
			getFluidBodyAttemptListSaveFileName(); 

		// Stops loading process if fluid body file doesn't exist; 
		if (!std::filesystem::exists(pathname))
			return false;

		const PairVector& pairs = loadPairedFile(pathname.string()); 

		FluidBodyAttempt fluidBodyAttempt;

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewFluidBodySpawnAttempt") {
				fluidBodyAttemptList.push_back(fluidBodyAttempt);
				fluidBodyAttempt = FluidBodyAttempt();
			}
			else if (attribute == "xpos")
				fluidBodyAttempt.position.x = std::stoi(value);
			else if (attribute == "ypos")
				fluidBodyAttempt.position.y = std::stoi(value);
			else if (attribute == "blockId")
				fluidBodyAttempt.fluidId = static_cast<Block::Id>(
					std::stoi(value)
				);
			else if (attribute == "sizeRangeX")
				fluidBodyAttempt.fluidSizeRange.x = std::stoi(value);
			else if (attribute == "sizeRangeY")
				fluidBodyAttempt.fluidSizeRange.y = std::stoi(value); 
		}

		return true; 
	}

	void World::saveWorldProperties() {
		PairVector pairs;

		// Adds world properties to the save file. 
		pairs.emplace_back("name", name);
		pairs.emplace_back("versionNumber", toString(versionNumber));
		pairs.emplace_back("seed", toString(seed));
		pairs.emplace_back("gameTime", toString(gameTime.gameTicks));

		savePairedFile(getSaveFileName(), pairs);
	}
	void World::saveChunk(const Chunk& chunk) {
		std::ofstream ofile(getChunkSaveFileName(chunk.offset));

		// Save property information. 
		ofile << "Properties\n"; 
		ofile << "biome = " << chunk.getBiomeId() << "\n";

		// Save block tile data. 
		ofile << "\nBlockData\n"; 

		for (int ypos = 0; ypos < Chunk::height; ypos++) {
			for (int xpos = 0; xpos < Chunk::width; xpos++) {
				const Block block = chunk.getBlock(xpos, ypos); 
				ofile << block.id << ":" << block.tags.asInt << " "; 
			}

			ofile << "\n"; 
		}

		// Save wall tile data. 
		ofile << "\nWallData\n"; 

		for (int ypos = 0; ypos < Chunk::height; ypos++) {
			for (int xpos = 0; xpos < Chunk::width; xpos++) {
				const Wall wall = chunk.getWall(xpos, ypos); 
				ofile << wall.id << ":" << wall.tags.asInt<< " "; 
			}

			ofile << "\n"; 
		}

		ofile.close(); 
	}
	void World::saveChunkEntities(const Chunk& chunk) {
		std::ofstream ofile(getChunkEntitySaveFileName(chunk.offset));

		// Save tile entity data. 
		ofile << "\nTileEntity\n";

		const TileEntity* tileEntity = nullptr;
		int tileEntityIndex = 0;

		do {
			tileEntity = chunk.getTileEntity(tileEntityIndex);

			if (tileEntity != nullptr) 
				ofile << "\n" << tileEntity->getSaveString() << "\n"; 

			tileEntityIndex++; 
		}
		while (tileEntity != nullptr);

		// Save entity data. 
		ofile << "\nEntity\n";

		for (int entityPairIndex = 1; entityPairIndex < entities.size();
			entityPairIndex++)
		{
			const EntityPair& entityPair = entities[entityPairIndex];

			Entity::Type entityType = entityPair.first;
			Entity* entity = entityPair.second;

			if (getChunkOffset(entity->position.x) == chunk.offset) {
				// True if the entity selected should be saved to an entity
				// file. Note: Some mobs such as hostile mobs won't be saved. 
				bool saveEntity = true; 

				switch (entityType) {
				case Entity::Type::Mob:
				{
					const Mob& mob = dynamic_cast<Mob&>(*entity);
					saveEntity = mob.behavior != Mob::Behavior::Hostile; 
				}
					break; 
				case Entity::Type::ProjectileEntity:
					saveEntity = false; 
					break; 
				}

				if (saveEntity)
					ofile << "\n" << entity->getSaveString() << "\n";

				delete entity; 
				entities.erase(entities.begin() + entityPairIndex);
				entityPairIndex--;
			}
		}

		ofile.close();
	}
	void World::savePlayer() {
		std::ofstream ofile(getPlayerSaveFileName());

		ofile << player->getSaveString() << "\n";

		ofile.close();
	}
	void World::saveTileList() {
		PairVector pairs;

		for (auto& tilePlacement : tileList) {
			// Adds world properties to the save file. 
			pairs.emplace_back("New Tile", "");
			pairs.emplace_back("xpos", toString(tilePlacement.position.x));
			pairs.emplace_back("ypos", toString(tilePlacement.position.y));
			pairs.emplace_back("blockId", toString(tilePlacement.block.id));
			pairs.emplace_back("blockTags", toString(tilePlacement.block.tags.asInt));
			pairs.emplace_back("wallId", toString(tilePlacement.wall.id));
			pairs.emplace_back("wallTags", toString(tilePlacement.wall.tags.asInt));
			pairs.emplace_back("placeFilter", toString(static_cast<int>(tilePlacement.placeFilter)));
			pairs.emplace_back("useBlock", toString(tilePlacement.useBlock));
		}

		pairs.emplace_back("New Tile", "");

		savePairedFile(getTileListSaveFileName(), pairs);
	}
	void World::saveFluidBodyAttemptList() {
		PairVector pairs; 

		for (auto& fluidBodyAttempt : fluidBodyAttemptList) {
			pairs.emplace_back("New Fluid Body Spawn Attempt", "");
			pairs.emplace_back("xpos", toString(fluidBodyAttempt.position.x)); 
			pairs.emplace_back("ypos", toString(fluidBodyAttempt.position.y)); 
			pairs.emplace_back("blockId", toString(fluidBodyAttempt.fluidId)); 
			pairs.emplace_back(
				"sizeRangeX", toString(fluidBodyAttempt.fluidSizeRange.x)
			); 
			pairs.emplace_back(
				"sizeRangeY", toString(fluidBodyAttempt.fluidSizeRange.y)
			); 
		}

		pairs.emplace_back("New Fluid Body Spawn Attempt", ""); 

		savePairedFile(getFluidBodyAttemptListSaveFileName(), pairs); 
	}
	
	bool World::isValidBlock(gs::Vec2i position, Block::Id blockId) {
		auto isBlockIdInArray = [](Block::Id blockId,
			const Block::Id* blockIds, int blockIdCount) -> bool
		{
			for (int blockIdIndex = 0; blockIdIndex < blockIdCount;
				blockIdIndex++)
			{
				if (blockIds[blockIdIndex] == blockId)
					return true;
			}

			return false; 
		};

		// Retrieves block id from position if not given. 
		if (blockId == Block::Invalid)
			blockId = getBlockId(position);

		// Local blocks 

		const Block blockAbove = getBlock(position + gs::Vec2i(0, -1));
		const Block blockBeneath = getBlock(position + gs::Vec2i(0, 1));
		const Block blockToLeft = getBlock(position + gs::Vec2i(-1, 0));
		const Block blockToRight = getBlock(position + gs::Vec2i(1, 0));

		bool isValid = true;

		switch (static_cast<BlockInfo::BlockDependencyType>(
			Block(blockId).getVar(BlockInfo::blockDependencyType)))
		{
		case BlockInfo::BlockDependencyType::Torch:
		{
			auto validTorchBlock = [](const Block block) -> bool {
				return !block.isEmpty() && !block.isFluid() && block.isSolid();
			};

			isValid = validTorchBlock(blockBeneath)
				|| validTorchBlock(blockToLeft)
				|| validTorchBlock(blockToRight)
				|| !getWall(position).isEmpty();
		}
			break;
		case BlockInfo::BlockDependencyType::GrassPlant:
		{
			const Block::Id validBlockIds[] = { 
				Block::Dirt, Block::GrassBlock
			};

			isValid = isBlockIdInArray(blockBeneath.id, validBlockIds, 2);
		}
			break;
		case BlockInfo::BlockDependencyType::SandPlant:
		{
			const Block::Id validBlockIds[] = { Block::Sand };

			isValid = isBlockIdInArray(blockBeneath.id, validBlockIds, 1);
		}
			break;
		case BlockInfo::BlockDependencyType::Cactus:
		{
			const Block::Id validBlockIds[] = {
				Block::Sand, Block::Cactus
			};

			isValid = isBlockIdInArray(blockBeneath.id, validBlockIds, 2);
		}
			break;
		case BlockInfo::BlockDependencyType::Bamboo:
		{
			const Block::Id validBlockIds[] = {
				Block::Dirt, Block::GrassBlock, Block::BambooStalk
			}; 

			isValid = isBlockIdInArray(blockBeneath.id, validBlockIds, 3);
		}
			break; 
		case BlockInfo::BlockDependencyType::Crop:
		{
			const Block::Id validBlockIds[] = { Block::FarmLand }; 

			isValid = isBlockIdInArray(blockBeneath.id, validBlockIds, 1);
		}
			break; 
		}

		return isValid;
	}
	void World::triggerBlockUpdates(gs::Vec2i position) {
		if (!blockUpdatesEnabled)
			return; 

		// Triggers updates for all blocks sorounding the current block. 
		for (int xpos = position.x - 1; xpos <= position.x + 1; xpos++) {
			for (int ypos = position.y - 1; ypos <= position.y + 1; ypos++) {
				if (isValidYpos(ypos)) {
					getBlockRef(xpos, ypos).updateState =
						Block::UpdateState::UpdateNext;
				}
			}
		}
	}
	int World::getVerticalPlantHeight(gs::Vec2i position, Block::Id blockId) {
		int height = 0; 

		// Keeps moving down the plant until it stops. 
		while (isValidYpos(position.y) && getBlock(position).id == blockId) {
			height++; 
			position.y++;
		}

		return height; 
	}
	void World::updateBlocks() {
		// Calculates the horizontal area that needs to be updated. 
		const gs::Vec2i horizontalBlockUpdateRange = gs::Vec2i(
			player->position.x - blockUpdateRange.x,
			player->position.x + blockUpdateRange.x
		);
		// Calculates the vertical area that needs to be updated. Note: It is 
		// automatically constrained to the size of the chunk. 
		const gs::Vec2i verticalBlockUpdateRange = gs::Vec2i(
			gs::util::clamp(static_cast<int>(player->position.y) 
				- blockUpdateRange.y, 0, Chunk::height),
			gs::util::clamp(static_cast<int>(player->position.y)
				+ blockUpdateRange.y, 0, Chunk::height)
		);

		gs::Vec2i blockPosition; 

		blocksUpdated = 0; 

		for (blockPosition.x = horizontalBlockUpdateRange.x; blockPosition.x 
			< horizontalBlockUpdateRange.y; blockPosition.x++)
		{
			for (blockPosition.y = verticalBlockUpdateRange.x; blockPosition.y 
				< verticalBlockUpdateRange.y; blockPosition.y++)
			{
				Block& block = getBlockRef(blockPosition); 

				const BlockInfo::BlockUpdate blockUpdate =
					static_cast<BlockInfo::BlockUpdate>(block.getVar(
						BlockInfo::blockUpdate)); 

				// Ensures that blocks that require tile-entities have them 
				// placed down. 
				if (block.getVar(BlockInfo::requiresTileEntity)) {
					TileEntity* tileEntity = getTileEntity(blockPosition); 

					// Creates tile-entity if none was found at the current 
					// position. 
					if (tileEntity == nullptr) 
						createTileEntity(blockPosition);
					else 
						tileEntity->update(block); 
				}
				if (block.tags.naturalBlock) {
					// Ensures naturally generated grass blocks aren't covered
					// by other solid blocks, if they are, turn them to dirt. 
					switch (block.id) {
					case Block::GrassBlock:
						if (getBlock(blockPosition + gs::Vec2i(0, -1)).isSolid())
							block.id = Block::Dirt;
						break; 
					case Block::BambooStalk:
						block.updateState = Block::UpdateState::NeedsUpdate; 
						break; 
					} 

					block.tags.naturalBlock = false; 
				}

				switch (blockUpdate) {
				case BlockInfo::BlockUpdate::Leaves:
					if (randomGenerator.generate()
						% (render::window::framerate * 30) == 0)
					{
						const int maxLeafLogPersistanceDistance = 4;
						const Block::Id logId = static_cast<Block::Id>(
							static_cast<int>(block.id) - 1
						);

						PathFinder pathFinder;

						pathFinder.startPoint = blockPosition;
						pathFinder.restrictions.left = blockPosition.x
							- maxLeafLogPersistanceDistance;
						pathFinder.restrictions.top = blockPosition.y
							- maxLeafLogPersistanceDistance;
						pathFinder.restrictions.width = blockPosition.x
							+ maxLeafLogPersistanceDistance;
						pathFinder.restrictions.height = blockPosition.y
							+ maxLeafLogPersistanceDistance;
						pathFinder.setSafeBlockFilter([](Block block) -> bool {
							auto isValidTreeBlock = [&](Block::Id treeLeaf)
								-> bool
							{
								return block.id == treeLeaf
									|| block.id == (treeLeaf - 1);
							};

							return isValidTreeBlock(Block::OakLeaves)
								|| isValidTreeBlock(Block::BirchLeaves)
								|| isValidTreeBlock(Block::SpruceLeaves)
								|| isValidTreeBlock(Block::JungleLeaves)
								|| isValidTreeBlock(Block::AcaciaLeaves);
						});

						if (!pathFinder.isBlockLocatable(logId)) 
							breakBlock(blockPosition);
					}
					break;
				case BlockInfo::BlockUpdate::Grass:
					if (randomGenerator.generate()
						% (render::window::framerate * 30) == 0)
					{
						// Turns grass-block into a dirt block if covered by 
						// solid block. 
						if (static_cast<collision::CollisionType>(getBlock(
							blockPosition + gs::Vec2i(0, -1)).getVar(
								BlockInfo::collisionType))
							== collision::CollisionType::Block)
						{
							setBlock(blockPosition, Block(Block::Dirt));
							break;
						}
					}

					// Transfers grass to nearby dirt blocks. 
					for (int xpos = -2; xpos <= 2; xpos++) {
						for (int ypos = -1; ypos <= 1; ypos++) {
							const gs::Vec2i translatedBlockPosition = gs::Vec2i(
								blockPosition.x + xpos, blockPosition.y + ypos
							);

							// Don't transfer to blocks in the same column. 
							if (xpos == 0) continue;

							if (randomGenerator.generate()
								% (render::window::framerate * 30) == 0)
							{
								// Only transfers grass to dirt blocks that
								// aren't covered by a solid block. 
								if (getBlockId(translatedBlockPosition) == Block::Dirt
									&& static_cast<collision::CollisionType>(getBlock(
										translatedBlockPosition + gs::Vec2i(0, -1)).getVar(
											BlockInfo::collisionType)) != collision::CollisionType::Block)
								{
									setBlockId(translatedBlockPosition, Block::GrassBlock);
									getBlockRef(translatedBlockPosition).tags.rotation = 0;
								}
							}
						}
					}
					break;
				case BlockInfo::BlockUpdate::Bamboo:
					if (randomGenerator.generate()
						% (render::window::framerate * 120) == 0)
					{
						// Grows bamboo if block above is empty. 
						if (getBlock(blockPosition + gs::Vec2i(0, -1)).isEmpty()) {
							if (getVerticalPlantHeight(blockPosition, Block::BambooStalk) < 15)
								placeBlock(blockPosition + gs::Vec2i(0, -1),
									Block(Block::BambooStalk));
						}
					}
					break;
				case BlockInfo::BlockUpdate::Sapling:
					if (randomGenerator.generate()
						% (render::window::framerate * 12) == 0)
					{
						// The determined type of the tree to be generated,
						// based on the sapling type. 
						const TreeType treeType =
							block.id == Block::OakSapling
								? TreeType::Oak :
							(block.id == Block::BirchSapling
								? TreeType::Birch :
							(block.id == Block::SpruceSapling
								? TreeType::Spruce :
							(block.id == Block::JungleSapling
								? TreeType::Jungle :
							(block.id == Block::AcaciaSapling
								? TreeType::Acacia :
							TreeType::None))));

						breakBlock(blockPosition, false); 
						generateTree(blockPosition, treeType, *this);
					}
					break; 
				case BlockInfo::BlockUpdate::Crop:
					if (randomGenerator.generate()
						% (render::window::framerate * 60) == 0)
					{
						block.tags.animationOffset++; 

						const int adultRolloverValue = 
							block.id == Block::Wheat ? 7 : 3; 

						if (block.tags.animationOffset == adultRolloverValue)
							placeBlock(blockPosition, Block(
								static_cast<Block::Id>(block.id 
									+ adultRolloverValue))); 
					}
					break; 
				case BlockInfo::BlockUpdate::FarmLand:
					if (randomGenerator.generate()
						% (render::window::framerate * 10) == 0)
					{
						const int maxFarmLandWaterDistance = 4;

						PathFinder pathFinder;

						pathFinder.startPoint = blockPosition;
						pathFinder.restrictions.left = blockPosition.x
							- maxFarmLandWaterDistance;
						pathFinder.restrictions.top = blockPosition.y;
						pathFinder.restrictions.width = blockPosition.x
							+ maxFarmLandWaterDistance;
						pathFinder.restrictions.height = blockPosition.y
							+ maxFarmLandWaterDistance;
						pathFinder.setSafeBlockFilter([](Block block) -> bool {
							return block.id == Block::Dirt 
								|| block.id == Block::GrassBlock
								|| block.id == Block::FarmLand
								|| block.id == Block::Water;
						});

						if (pathFinder.isBlockLocatable(Block::Water))
							block.tags.animationOffset = 1;
						else 
							placeBlock(blockPosition, Block(Block::Dirt)); 
					}

					break; 
				}

				if (block.updateState == Block::UpdateState::NeedsUpdate) {
					bool applyBlockUpdate = true; 

					if (static_cast<BlockInfo::BlockDependencyType>(
						block.getVar(BlockInfo::blockDependencyType))
						!= BlockInfo::BlockDependencyType::None)
					{ 
						block.updateState = Block::UpdateState::NoUpdate;

						// If a block is no longer valid, such as a floating plant,
						// it is broken.  
						if (!isValidBlock(blockPosition)) {
							breakBlock(blockPosition);
							applyBlockUpdate = false; 
						}
					}

					if (applyBlockUpdate) {
						const gs::Vec2i blockAbovePosition = blockPosition 
							+ gs::Vec2i(0, -1);
						const gs::Vec2i blockBeneathPosition = blockPosition
							+ gs::Vec2i(0, 1);
						const gs::Vec2i blockToLeftPosition = blockPosition
							+ gs::Vec2i(-1, 0);
						const gs::Vec2i blockToRightPosition = blockPosition
							+ gs::Vec2i(1, 0);

						const Block blockAbove = getBlock(blockAbovePosition);
						const Block blockBeneath = getBlock(blockBeneathPosition);
						const Block blockToLeft = getBlock(blockToLeftPosition);
						const Block blockToRight = getBlock(blockToRightPosition);

						switch (blockUpdate) {
						case BlockInfo::BlockUpdate::FallingBlock:
							// Update block falling 10 times per second. 
							if (render::window::ticks % (render::window::framerate / 10) == 0) {
								// If the block beneath a falling block is empty, 
								// replace it with the block above and remove the
								// original block. 
								if (blockBeneath.isEmpty()) {
									placeBlock(blockBeneathPosition, block);
									breakBlock(blockPosition, false);
								}

								block.updateState = Block::UpdateState::NoUpdate;
							}

							break;
						case BlockInfo::BlockUpdate::Torch:
						{
							// Calculates which face the torch should stick to. 
							if (!blockBeneath.isEmpty() && !blockBeneath.isFluid()
									&& blockBeneath.isSolid())
								block.tags.animationOffset = 0;
							else if (!blockToLeft.isEmpty() && !blockToLeft.isFluid()
									&& blockToLeft.isSolid())
								block.tags.animationOffset = 2;
							else if (!blockToRight.isEmpty() && !blockToRight.isFluid()
									&& blockToRight.isSolid())
								block.tags.animationOffset = 3;
							else
								block.tags.animationOffset = 1;

							block.updateState = Block::UpdateState::NoUpdate;
						}
							break;
						case BlockInfo::BlockUpdate::Fluid:
						{
							const int maxFluidLevel = 7; 
							auto getLowestWaterLevel = [&](gs::Vec2i position) -> int {
								const gs::Vec2i neighbors[4] = {
									gs::Vec2i(0, -1), gs::Vec2i(1, 0),
									gs::Vec2i(0, 1), gs::Vec2i(-1, 0)
								};

								int lowestWaterLevel = maxFluidLevel;

								for (int neighborIndex = 0; neighborIndex < 4;
									neighborIndex++)
								{
									const gs::Vec2i neighborPosition = position
										+ neighbors[neighborIndex];
									const Block neighborBlock = getBlock(neighborPosition);

									if (neighborBlock.isFluid())
										lowestWaterLevel = std::min(
											lowestWaterLevel,
											static_cast<int>(neighborBlock.tags.fluidLevel)
										);
								}

								return lowestWaterLevel;
							};
							auto hasLowestWaterLevel = [&](gs::Vec2i position) -> bool {
								// Compares water level of tile at position compared
								// to it's neighbors. 
								return world->getBlock(position).tags.fluidLevel
									<= getLowestWaterLevel(position);
							};	

							// Update fluids 6 times per second. 
							if (render::window::ticks % (render::window::framerate / 6) == 0) {
								Block fluid(block.id, 0ull); 

								if (!block.tags.isFluidSource) {
									if (block.id == Block::Water
										&& blockToLeft.id == Block::Water
										&& blockToLeft.tags.isFluidSource 
										&& blockToRight.id == Block::Water 
										&& blockToRight.tags.isFluidSource)
									{
										block.tags.isFluidSource = true;
										block.tags.fluidLevel = 0;
										break; 
									}
									else if (!blockAbove.isFluid()) {
										if (hasLowestWaterLevel(blockPosition)
											|| (!hasLowestWaterLevel(blockPosition) 
												&& blockBeneath.isFluid()
												&& !blockToLeft.isFluid()
												&& !blockToRight.isFluid()))
										{
											breakBlock(blockPosition, false);
											break;
										}
									}
								}
								else {
									const gs::Vec2i blockToFarLeftPosition =
										blockPosition + gs::Vec2i(-2, 0); 
									const gs::Vec2i blockToFarRightPosition =
										blockPosition + gs::Vec2i(2, 0); 

									const Block blockToFarLeft = 
										getBlock(blockToFarLeftPosition); 
									const Block blockToFarRight =
										getBlock(blockToFarRightPosition);

									Block fluidSource = fluid; 
									fluidSource.tags.isFluidSource = true; 

									if (blockToLeft.isFluidBreakable()) {
										if (blockToFarLeft.isFluid()
											&& blockToFarLeft.tags.isFluidSource)
										{
											breakBlock(blockToLeftPosition, true, true);
											placeBlock(blockToLeftPosition, fluidSource);
										}
										else {
											fluidSource.tags.isFluidSource = false;
											fluidSource.tags.fluidLevel = 1;
											breakBlock(blockToLeftPosition, true, true);
											placeBlock(blockToLeftPosition, fluidSource);
										} 
									}

									if (blockToRight.isFluidBreakable()) {
										if (blockToFarRight.isFluid()
											&& blockToFarRight.tags.isFluidSource)
										{
											breakBlock(blockToRightPosition, true, true);
											placeBlock(blockToRightPosition, fluidSource);
										}
										else {
											fluidSource.tags.isFluidSource = false; 
											fluidSource.tags.fluidLevel = 1; 
											breakBlock(blockToRightPosition, true, true);
											placeBlock(blockToRightPosition, fluidSource);
										}
									}
								}

								if ((blockBeneath.isFluidBreakable() || blockBeneath.isFluid())
									&& !blockBeneath.tags.isFluidSource) 
								{	
									if (!blockBeneath.isFluid())
										breakBlock(blockBeneathPosition, true, true);

									setBlock(blockBeneathPosition, fluid); 
									getBlockRef(blockBeneathPosition).updateState =
										Block::UpdateState::UpdateNext; 
								}
								else if (block.tags.fluidLevel < maxFluidLevel
									&& !blockBeneath.isEmpty() 
									&& !blockBeneath.isFluid()) 
								{
									fluid.tags.fluidLevel = block.tags.fluidLevel + 1; 
									 
									if (blockToLeft.isFluidBreakable()) {
										breakBlock(blockToLeftPosition, true, true);
										placeBlock(blockToLeftPosition, fluid);
									}
									else if (blockToLeft.isFluid()) {
										Block& blockToLeftRef = getBlockRef(blockToLeftPosition); 
										blockToLeftRef.updateState = Block::UpdateState::NeedsUpdate; 
										blockToLeftRef.tags.fluidLevel = std::min(
											blockToLeft.tags.fluidLevel, fluid.tags.fluidLevel
										); 
									}

									if (blockToRight.isFluidBreakable()) {
										breakBlock(blockToRightPosition, true, true);
										placeBlock(blockToRightPosition, fluid);
									}
									else if (blockToRight.isFluid()) {
										Block& blockToRightRef = getBlockRef(blockToRightPosition);
										blockToRightRef.updateState = Block::UpdateState::UpdateNext;
										blockToRightRef.tags.fluidLevel = std::min(
											blockToRight.tags.fluidLevel, fluid.tags.fluidLevel
										);
									}
								}

								block.updateState = Block::UpdateState::NoUpdate; 
							}
						}
							break; 
						case BlockInfo::BlockUpdate::Bamboo:
							// Changes bamboo texture depending on the height
							// of the bamboo-stalk. s
							if (blockAbove.id == Block::Id::BambooStalk
									|| blockBeneath.id == Block::Id::BambooStalk)
								block.tags.animationOffset = 1 
									+ (std::abs(blockPosition.x + blockPosition.y) % 3); 

							block.updateState = Block::UpdateState::NoUpdate; 
							break; 
						case BlockInfo::BlockUpdate::FarmLand:
							if (blockAbove.isSolid())
								placeBlock(blockPosition, Block(Block::Dirt)); 
							break; 
						default:
							block.updateState = Block::UpdateState::NoUpdate;
							break;
						}
					}

					blocksUpdated++; 
				}
			}
		}

		// Prepare for the next update call. 
		for (blockPosition.x = horizontalBlockUpdateRange.x; blockPosition.x 
			< horizontalBlockUpdateRange.y; blockPosition.x++)
		{
			for (blockPosition.y = verticalBlockUpdateRange.x; blockPosition.y 
				< verticalBlockUpdateRange.y; blockPosition.y++)
			{
				Block& block = getBlockRef(blockPosition);

				if (block.updateState == Block::UpdateState::UpdateNext)
					block.updateState = Block::UpdateState::NeedsUpdate;
			}
		}
	}

	Chunk* World::getChunkSafely(int chunkOffset) {
		for (auto& chunk : chunks) {
			if (chunk->offset == chunkOffset)
				return chunk.get();
		}

		return nullptr;
	}

	std::string World::getSaveFileName() const {
		return saveFileDirectory + "/world.sav";
	}
	std::string World::getChunkSaveFileName(int chunkIndex) const {
		return saveFileDirectory + "/chunks/chunk"
			+ toString(chunkIndex) + ".sav"; 
	}
	std::string World::getChunkEntitySaveFileName(int chunkIndex) const {
		return saveFileDirectory + "/chunks/entity" 
			+ toString(chunkIndex) + ".sav";
	}
	std::string World::getPlayerSaveFileName() const {
		return saveFileDirectory + "/player.sav";
	}
	std::string World::getTileListSaveFileName() const {
		return saveFileDirectory + "/tileList.sav"; 
	}
	std::string World::getFluidBodyAttemptListSaveFileName() const {
		return saveFileDirectory + "/fluidBodyAttemptList.sav";
	}
	std::string World::getIconSaveFileName() const {
		return saveFileDirectory + "/icon.png"; 
	}

	gs::Vec2i World::getChunkPosition(gs::Vec2i position) {
		return gs::Vec2i(gs::util::mod(position.x, Chunk::width), position.y);
	}
	bool World::isValidYpos(int ypos) {
		return ypos >= 0 && ypos < Chunk::height;
	}

	World* world = nullptr; 
}