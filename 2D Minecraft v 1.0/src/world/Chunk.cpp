#include "../../hdr/world/Chunk.hpp"
#include "../../hdr/world/Generation.hpp"
#include "../../hdr/graphics/lighting/Lighting.hpp"

namespace engine {
	Chunk::Chunk() : 
		offset(0),
		loadedFromSave(false), 
		needsToBeSaved(true)
	{
	}
	Chunk::Chunk(int offset) : offset(offset) {
	}
	Chunk::Chunk(Biome biome) {
		setBiome(biome);
	}
	Chunk::Chunk(Biome::Id biomeId) {
		setBiomeId(biomeId); 
	}
	Chunk::Chunk(int offset, Biome biome) : offset(offset) {
		setBiome(biome); 
	}
	Chunk::Chunk(int offset, Biome::Id biomeId) : offset(offset) {
		setBiomeId(biomeId); 
	}

	void Chunk::addTileEntity(TileEntity tileEntity) {
		tileEntities.push_back(tileEntity);
	}
	void Chunk::createTileEntity(gs::Vec2i position) {
		addTileEntity(TileEntity(position));
	}
	void Chunk::createTileEntity(int xpos, int ypos) {
		createTileEntity({ xpos, ypos });
	}
	void Chunk::removeTileEntity(gs::Vec2i position) {
		for (int tileEntityIndex = 0; tileEntityIndex < tileEntities.size(); 
			tileEntityIndex++) 
		{
			const TileEntity* tileEntity = getTileEntity(tileEntityIndex); 

			// If tile entity matches the position given, erase it from the 
			// vector. 
			if (tileEntity->position == position) {
				tileEntities.erase(tileEntities.begin() + tileEntityIndex); 
				return; 
			}
		}
	}
	void Chunk::removeTileEntity(int xpos, int ypos) {
		removeTileEntity({ xpos, ypos });
	}

	void Chunk::setBlock(gs::Vec2i position, Block block) {
		blocks[position.x][position.y] = block;
		needsToBeSaved = true; 
	}
	void Chunk::setBlock(int xpos, int ypos, Block block) {
		setBlock({ xpos, ypos }, block);
	}
	void Chunk::setBlockId(gs::Vec2i position, Block::Id blockId) {
		blocks[position.x][position.y].id = blockId;
		needsToBeSaved = true;
	}
	void Chunk::setBlockId(int xpos, int ypos, Block::Id blockId) {
		setBlockId({ xpos, ypos }, blockId);
	}
	void Chunk::setWall(gs::Vec2i position, Wall wall) {
		walls[position.x][position.y] = wall;
		needsToBeSaved = true;
	}
	void Chunk::setWall(int xpos, int ypos, Wall wall) {
		setWall({ xpos, ypos }, wall);
	}
	void Chunk::setWallId(gs::Vec2i position, Wall::Id wallId) {
		walls[position.x][position.y].id = wallId;
		needsToBeSaved = true;
	}
	void Chunk::setWallId(int xpos, int ypos, Wall::Id wallId) {
		setWallId({ xpos, ypos }, wallId);
	}
	void Chunk::setTileColor(gs::Vec2i position, TileColor tileColor) {
		tileColors[position.x][position.y] = tileColor;
	}
	void Chunk::setTileColor(int xpos, int ypos, TileColor tileColor) {
		setTileColor({ xpos, ypos }, tileColor);
	}
	void Chunk::setBiome(Biome biome) {
		this->biome = biome; 
		needsToBeSaved = true;
	}
	void Chunk::setBiomeId(Biome::Id biomeId) {
		biome.id = biomeId; 
		needsToBeSaved = true;
	}

	Block Chunk::getBlock(gs::Vec2i postion) const {
		return blocks[postion.x][postion.y];
	}
	Block Chunk::getBlock(int xpos, int ypos) const {
		return getBlock({ xpos, ypos });
	}
	Block& Chunk::getBlockRef(gs::Vec2i position) {
		return blocks[position.x][position.y]; 
	}
	Block& Chunk::getBlockRef(int xpos, int ypos) {
		return getBlockRef({ xpos, ypos }); 
	}
	Block::Id Chunk::getBlockId(gs::Vec2i position) const {
		return blocks[position.x][position.y].id;
	}
	Block::Id Chunk::getBlockId(int xpos, int ypos) const {
		return getBlockId({ xpos, ypos });
	}
	Wall Chunk::getWall(gs::Vec2i position) const {
		return walls[position.x][position.y];
	}
	Wall Chunk::getWall(int xpos, int ypos) const {
		return getWall({ xpos, ypos });
	}
	Wall& Chunk::getWallRef(gs::Vec2i position) {
		return walls[position.x][position.y]; 
	}
	Wall& Chunk::getWallRef(int xpos, int ypos) {
		return getWallRef({ xpos, ypos }); 
	}
	Wall::Id Chunk::getWallId(gs::Vec2i position) const {
		return walls[position.x][position.y].id; 
	}
	Wall::Id Chunk::getWallId(int xpos, int ypos) const {
		return getWallId({ xpos, ypos }); 
	}
	TileEntity* Chunk::getTileEntity(gs::Vec2i position) {
		for (auto& tileEntity : tileEntities) {
			if (tileEntity.position == position)
				return &tileEntity; 
		}

		return nullptr; 
	}
	TileEntity* Chunk::getTileEntity(int xpos, int ypos) {
		return getTileEntity(xpos, ypos); 
	}
	const TileEntity* Chunk::getTileEntity(int index) const {
		if (index < 0 || index >= tileEntities.size())
			return nullptr; 

		return &tileEntities[index];
	}
	TileColor Chunk::getTileColor(gs::Vec2i position) const {
		return tileColors[position.x][position.y];
	}
	TileColor Chunk::getTileColor(int xpos, int ypos) const {
		return getTileColor({ xpos, ypos }); 
	}
	Biome Chunk::getBiome() const {
		return biome; 
	}
	Biome::Id Chunk::getBiomeId() const {
		return biome.id; 
	}

	void Chunk::clear() {
		for (int xpos = 0; xpos < width; xpos++) {
			for (int ypos = 0; ypos < height; ypos++) {
				setBlockId(xpos, ypos, Block::Air);
				setWallId(xpos, ypos, Wall::Air); 
				setTileColor(xpos, ypos, TileColor::White);
			}
		}
	}
}