#pragma once

// Dependencies
#include "Tile.hpp"
#include "TileEntity.hpp"
#include "Biome.hpp"
#include "../graphics/lighting/Light.hpp"

namespace engine {
	class Chunk {
	public:
		int offset; 
		bool loadedFromSave; 
		bool needsToBeSaved; 

		Chunk(); 
		Chunk(int offset); 
		Chunk(Biome biome); 
		Chunk(Biome::Id biomeId); 
		Chunk(int offset, Biome biome);
		Chunk(int offset, Biome::Id biomeId); 
		~Chunk() = default; 

		void addTileEntity(TileEntity tileEntity);
		void createTileEntity(gs::Vec2i position); 
		void createTileEntity(int xpos, int ypos); 
		void removeTileEntity(gs::Vec2i position); 
		void removeTileEntity(int xpos, int ypos); 

		void setBlock(gs::Vec2i position, Block block); 
		void setBlock(int xpos, int ypos, Block block); 
		void setBlockId(gs::Vec2i position, Block::Id blockId);
		void setBlockId(int xpos, int ypos, Block::Id blockId);
		void setWall(gs::Vec2i position, Wall wall); 
		void setWall(int xpos, int ypos, Wall wall); 
		void setWallId(gs::Vec2i position, Wall::Id wallId); 
		void setWallId(int xpos, int ypos, Wall::Id wallId); 
		void setTileColor(gs::Vec2i position, TileColor tileColor); 
		void setTileColor(int xpos, int ypos, TileColor tileColor); 
		void setBiome(Biome biome); 
		void setBiomeId(Biome::Id biomeId); 

		Block getBlock(gs::Vec2i postion) const; 
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
		const TileEntity* getTileEntity(int index) const;
		TileColor getTileColor(gs::Vec2i position) const; 
		TileColor getTileColor(int xpos, int ypos) const; 
		Biome getBiome() const; 
		Biome::Id getBiomeId() const; 

		static constexpr int width = 16;
		static constexpr int height = 512;  
	private:
		Block blocks[width][height]; 
		Wall walls[width][height]; 
		std::vector<TileEntity> tileEntities; 
		TileColor tileColors[width][height]; 
		Biome biome; 

		void clear();
	};
}