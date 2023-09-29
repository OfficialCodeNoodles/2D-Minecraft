#pragma once

// Dependencies
#include "Chunk.hpp"
#include "GameTime.hpp"
#include "../inventory/LootTable.hpp"

namespace engine {
	class World {
	public:
		enum class PlaceFilter { Replace, Fill };

		std::string folderName, name;
		float versionNumber; 
		int seed;
		GameTime gameTime; 

		World(); 
		~World(); 

		void createWorld(const std::string& folderName, const std::string& worldName = "UseFolderName");
		void loadWorld(const std::string& folderName); 
		void saveWorld();
		void saveIcon();

		void generateSeed(); 
		void update(); 
		void enableBlockUpdates(bool enable = true);

		bool isValidBlockPlacementLocation(gs::Vec2i position, Block::Id blockId); 
		bool isValidBlockPlacementLocation(int xpos, int ypos, Block::Id blockId); 
		bool isValidWallPlacementLocation(gs::Vec2i position, Wall::Id wallId); 
		bool isValidWallPlacementLocation(int xpos, int ypos, Wall::Id wallId); 
		// Tries to set the block but if the chunk isn't loaded it gets put 
		// into a vector to be added when the chunk is finally loaded in. 
		void placeBlock(
			gs::Vec2i position, Block block, 
			PlaceFilter placeFilter = PlaceFilter::Replace, 
			bool playSoundEvent = false
		); 
		void placeBlock(
			int xpos, int ypos, Block block,
			PlaceFilter placeFilter = PlaceFilter::Replace,
			bool playSoundEvent = false
		);
		void placeBlock(
			gs::Vec2i position, Block::Id blockId, 
			PlaceFilter placeFilter = PlaceFilter::Replace,
			bool playSoundEvent = false
		);
		void placeBlock(
			int xpos, int ypos, Block::Id blockId, 
			PlaceFilter placeFilter = PlaceFilter::Replace,
			bool playSoundEvent = false
		);
		// Tries to set the wall but if the chunk isn't loaded it gets put 
		// into a vector to be added when the chunk is finally loaded in. 
		void placeWall(
			gs::Vec2i position, Wall wall, 
			PlaceFilter placeFilter = PlaceFilter::Replace,
			bool playSoundEvent = false
		);
		void placeWall(
			int xpos, int ypos, Wall wall,
			PlaceFilter placeFilter = PlaceFilter::Replace,
			bool playSoundEvent = false
		);
		void placeWall(
			gs::Vec2i position, Wall::Id wallId, 
			PlaceFilter placeFilter = PlaceFilter::Replace, 
			bool playSoundEvent = false
		);
		void placeWall(
			int xpos, int ypos, Wall::Id wallId, 
			PlaceFilter placeFilter = PlaceFilter::Replace,
			bool playSoundEvent = false
		);
		void breakBlock(
			gs::Vec2i position, bool dropItem = true, 
			bool playSoundEvent = false, bool generateParticles = true
		);
		void breakBlock(
			int xpos, int ypos, bool dropItem = true,
			bool playSoundEvent = false, bool generateParticles = true
		);
		void breakWall(
			gs::Vec2i position, bool dropItem = true,
			bool playSoundEvent = false, bool generateParticles = true
		);
		void breakWall(
			int xpos, int ypos, bool dropItem = true,
			bool playSoundEvent = false, bool generateParticles = true
		);
		void addTileEntity(TileEntity tileEntity); 
		void createTileEntity(gs::Vec2i position); 
		void createTileEntity(int xpos, int ypos); 
		void removeTileEntity(gs::Vec2i position); 
		void removeTileEntity(int xpos, int ypos); 
		void addFluidBodyAttempt(
			gs::Vec2i position, Block::Id fluidId, 
			gs::Vec2i sizeRange = gs::Vec2i(-1, -1)
		); 
			
		bool setBlock(gs::Vec2i position, Block block); 
		bool setBlock(int xpos, int ypos, Block block); 
		bool setBlockId(gs::Vec2i position, Block::Id blockId); 
		bool setBlockId(int xpos, int ypos, Block::Id blockId); 
		bool setWall(gs::Vec2i position, Wall wall); 
		bool setWall(int xpos, int ypos, Wall wall); 
		bool setWallId(gs::Vec2i position, Wall::Id wallId); 
		bool setWallId(int xpos, int ypos, Wall::Id wallId); 
		void setTileColor(gs::Vec2i position, TileColor tileColor); 
		void setTileColor(int xpos, int ypos, TileColor tileColor); 

		Block getBlock(gs::Vec2i position) const; 
		Block getBlock(int xpos, int ypos) const;
		Block& getBlockRef(gs::Vec2i position); 
		Block& getBlockRef(int xpos, int ypos); 
		Block::Id getBlockId(gs::Vec2i position) const;
		Block::Id getBlockId(int xpos, int ypos) const; 
		Wall getWall(gs::Vec2i position) const;
		Wall getWall(int xpos, int ypos) const; 
		Wall& getWallRef(gs::Vec2i position);
		Wall& getWallRef(int xpos, int ypos); 
		Wall::Id getWallId(gs::Vec2i position) const; 
		Wall::Id getWallId(int xpos, int ypos) const; 
		TileEntity* getTileEntity(gs::Vec2i position); 
		TileEntity* getTileEntity(int xpos, int ypos); 
		TileColor getTileColor(gs::Vec2i position) const; 
		TileColor getTileColor(int xpos, int ypos) const; 
		Chunk* getChunk(int chunkOffset) const;
		int getNumOfBlocksUpdated() const; 
		bool isBlockExposedToSky(gs::Vec2i position) const;
		bool isBlockExposedToSky(int xpos, int ypos) const; 

		static constexpr int chunkUnloadDistance = 25; 
		static constexpr int chunkLoadDistance = 10;
		static const gs::Vec2i blockUpdateRange; 

		// Finds the offset of a chunk, based on a global xpos. 
		static int getChunkOffset(int xpos); 
	private:
		struct TilePlacement {
			gs::Vec2i position;
			Block block;
			Wall wall;
			PlaceFilter placeFilter;
			bool useBlock;

			TilePlacement(); 
			~TilePlacement() = default; 
		};
		struct FluidBodyAttempt {
			gs::Vec2i position; 
			Block::Id fluidId; 
			gs::Vec2i fluidSizeRange; 

			FluidBodyAttempt(); 
			~FluidBodyAttempt() = default; 
		};

		std::string saveFileDirectory; 
		std::vector<std::unique_ptr<Chunk>> chunks; 
		std::vector<TilePlacement> tileList; 
		std::vector<FluidBodyAttempt> fluidBodyAttemptList; 
		int baseChunkOffset; 
		bool blockUpdatesEnabled; 
		int blocksUpdated; 

		void createWorldFileDirectories() const;

		void loadWorldProperties();
		bool loadChunk(Chunk& chunk); 
		bool loadChunkEntities(Chunk& chunk); 
		bool loadPlayer(); 
		bool loadTileList(); 
		bool loadFluidBodyAttemptList(); 
		void saveWorldProperties(); 
		void saveChunk(const Chunk& chunk); 
		void saveChunkEntities(const Chunk& chunk); 
		void savePlayer(); 
		void saveTileList(); 
		void saveFluidBodyAttemptList(); 

		bool isValidBlock(gs::Vec2i position, Block::Id blockId = Block::Invalid); 
		// Will cause the block along with it's neighbors to require updates. 
		void triggerBlockUpdates(gs::Vec2i position); 
		int getVerticalPlantHeight(gs::Vec2i position, Block::Id blockId); 
		void updateBlocks(); 

		// Performs a linear search for a chunk incase the chunks are unordered.
		Chunk* getChunkSafely(int chunkIndex); 
		std::string getSaveFileName() const;
		std::string getChunkSaveFileName(int chunkIndex) const; 
		std::string getChunkEntitySaveFileName(int chunkIndex) const;
		std::string getPlayerSaveFileName() const; 
		std::string getTileListSaveFileName() const; 
		std::string getFluidBodyAttemptListSaveFileName() const; 
		std::string getIconSaveFileName() const; 

		static gs::Vec2i getChunkPosition(gs::Vec2i position); 
		static bool isValidYpos(int ypos); 
	};

	extern World* world; 
}