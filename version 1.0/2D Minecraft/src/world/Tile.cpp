#include "../../hdr/world/Tile.hpp"
#include "../../hdr/util/Random.hpp"
#include "../../hdr/entity/Collision.hpp"
#include "../../hdr/graphics/UI.hpp"
#include "../../hdr/audio/AudioEnviroment.hpp"

namespace engine {
	Block::Tags::Tags() : asInt(0ull) {
	}

	Block::Block() : 
		id(Air),
		updateState(UpdateState::NoUpdate)
	{
	}
	Block::Block(Block::Id blockId) : 
		id(blockId),
		updateState(UpdateState::NoUpdate)
	{
		init(); 
	}
	Block::Block(TagInt tags) : 
		id(Air),
		updateState(UpdateState::NoUpdate)
	{
		this->tags.asInt = tags;
	}
	Block::Block(Block::Id blockId, TagInt tags) : 
		id(blockId),
		updateState(UpdateState::NoUpdate)
	{
		this->tags.asInt = tags; 
	}

	void Block::init() {
		const BlockInfo::BlockUpdate blockUpdate = 
			static_cast<BlockInfo::BlockUpdate>(getVar(BlockInfo::blockUpdate));
		const int randomRotation = getVar(BlockInfo::randomRotation);

		switch (blockUpdate) {
		case BlockInfo::BlockUpdate::Fluid:
			tags.isFluidSource = true; 
			break; 
		case BlockInfo::BlockUpdate::Bamboo:
			tags.naturalBlock = true;	 
			break; 
		}

		if (randomRotation != 0) {
			int rotation = 0; 

			switch (randomRotation) {
			case 1:
				rotation = (randomGenerator.generate() % 2) * 2;
				break; 
			case 2:
				rotation = randomGenerator.generate() % 4;
				break; 
			}

			tags.rotation = rotation; 
		}

		switch (id) {
		case CraftingTable:
			tags.animationOffset = randomGenerator.generate() % 2;
			break; 
		}
	}

	float Block::getVar(PropertyInt propertyInt) const {
		return BlockInfo::getVar(
			id, static_cast<BlockInfo::Property>(propertyInt)
		); 
	}

	bool Block::isEmpty() const {
		return id == Air; 
	}
	bool Block::isFluid() const {
		return static_cast<BlockInfo::BlockUpdate>(
			getVar(BlockInfo::blockUpdate)) == BlockInfo::BlockUpdate::Fluid; 
	}
	bool Block::isFluidBreakable() const {
		return static_cast<bool>(getVar(BlockInfo::fluidBreakable));
	}
	bool Block::isSolid() const {
		return static_cast<collision::CollisionType>(
			getVar(BlockInfo::collisionType)) == collision::CollisionType::Block; 
	}

	Wall::Tags::Tags() : asInt(0ull) {
	}

	Wall::Wall() : id(Air) {
	}
	Wall::Wall(Wall::Id wallId) : id(wallId){
	}
	Wall::Wall(TagInt tags) : id(Air) {
		this->tags.asInt = tags;	
	}
	Wall::Wall(Wall::Id wallId, TagInt tags) : id(wallId) {
		this->tags.asInt = tags; 
	}

	void Wall::init() {
	}

	float Wall::getVar(PropertyInt propertyInt) const {
		return WallInfo::getVar(
			id, static_cast<WallInfo::Property>(propertyInt)
		);
	}

	bool Wall::isEmpty() const {
		return id == Air; 
	}

	BlockInfo::BlockInfo() {
		setVar(lightIndex, 0);
		setVar(requiresTileEntity, false); 
		setVar(collisionType, static_cast<float>(collision::CollisionType::Block)); 
		setVar(blockUpdate, static_cast<float>(BlockUpdate::None)); 
		setVar(blockDependencyType, static_cast<float>(BlockDependencyType::None)); 
		setVar(randomRotation, 0); 
		setVar(hasTransparency, false); 
		setVar(hasBlockOverlay, false); 
		setVar(renderUnderside, false); 
		setVar(generationReplacable, false); 
		setVar(foreground, false); 
		setVar(requiredToolToBreak, static_cast<float>(ItemInfo::ToolType::None));
		setVar(requiredToolStrength, 0.0f); 
		setVar(hardness, 1.0f); 
		setVar(dropItemWithIncorrectTool, false); 
		setVar(inventoryMenu, static_cast<int>(render::ui::InventoryMenu::None)); 
		setVar(horizontalShift, false);
		setVar(applyAnimationLighting, false); 
		setVar(lootTable, static_cast<int>(LootTable::None)); 
		setVar(soundEvent, static_cast<int>(audio::SoundEvent::Stone)); 
	}

	void BlockInfo::setVar(Property property, float value) {
		vars[property] = value; 
	}
	float BlockInfo::getVar(Property property) const {
		return vars[property];
	}

	const std::string BlockInfo::propertyStrings[BlockInfo::End] = {
		"textureIndex",				"lightIndex",
		"requiresTileEntity",		"collisionType", 
		"blockUpdate",				"blockDependencyType", 
		"randomRotation",			"hasTransparency",
		"hasBlockOverlay",			"renderUnderside", 
		"generationReplacable",		"foreground", 
		"requiredToolToBreak",		"requiredToolStrength",
		"hardness",					"itemDrop",
		"dropItemWithIncorrectTool","inventoryMenu",
		"horizontalShift",			"applyAnimationLighting",
		"lootTable",				"soundEvent",
		"fluidBreakable"
	}; 
	BlockInfo BlockInfo::blockInfo[BlockInfo::numOfBlocks];

	float BlockInfo::getVar(Block::Id blockId, Property property) {
		return blockInfo[blockId].getVar(property);
	}

	WallInfo::WallInfo() {
		setVar(hasTransparency, false); 
		setVar(requiredToolToBreak, static_cast<float>(ItemInfo::ToolType::None));
		setVar(requiredToolStrength, 0.0f);
		setVar(hardness, 1.0f);
		setVar(dropItemWithIncorrectTool, false);
		setVar(soundEvent, static_cast<int>(audio::SoundEvent::Stone)); 
	}
	
	void WallInfo::setVar(Property property, float value) {
		vars[property] = value; 
	}
	float WallInfo::getVar(Property property) const {
		return vars[property]; 
	} 

	const std::string WallInfo::propertyStrings[WallInfo::End] = {
		"textureIndex",				"lightIndex",
		"hasTransparency",
		"requiredToolToBreak",		"requiredToolStrength",
		"hardness",					"itemDrop", 
		"dropItemWithIncorrectTool","soundEvent"
	};
	WallInfo WallInfo::wallInfo[WallInfo::numOfWalls]; 

	float WallInfo::getVar(Wall::Id wallId, Property property) {
		return wallInfo[wallId].getVar(property);
	}

	void loadBlockInfo() {
		const PairVector& pairs = loadPairedFile(
			render::assetDirectory + "data/block.list"
		);

		for (int blockInfoIndex = 0; blockInfoIndex < BlockInfo::numOfBlocks; 
			blockInfoIndex++) 
		{
			BlockInfo& blockInfo = BlockInfo::blockInfo[blockInfoIndex];

			blockInfo.setVar(BlockInfo::textureIndex, blockInfoIndex); 
			blockInfo.setVar(
				BlockInfo::itemDrop, blockInfoIndex + ItemInfo::defaultTileItems
			); 
		}

		Block::Id blockId = static_cast<Block::Id>(0);
		BlockInfo* blockInfo = nullptr; 

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewBlock") {
				blockId = static_cast<Block::Id>(std::stoi(value));
				blockInfo = &BlockInfo::blockInfo[blockId]; 
			}
			else if (blockInfo != nullptr && !value.empty()) {
				BlockInfo::Property property = BlockInfo::End; 

				for (int propertyIndex = 0; propertyIndex < BlockInfo::End;
					propertyIndex++)
				{
					if (BlockInfo::propertyStrings[propertyIndex] == attribute)
						property = static_cast<BlockInfo::Property>(propertyIndex); 
				}
				

				blockInfo->setVar(property, std::stof(value)); 
			}
		}
	}	
	void loadWallInfo() {
		const PairVector& pairs = loadPairedFile(
			render::assetDirectory + "data/wall.list"
		);

		for (int wallInfoIndex = 0; wallInfoIndex < WallInfo::numOfWalls;
			wallInfoIndex++)
		{
			WallInfo& wallInfo = WallInfo::wallInfo[wallInfoIndex];

			wallInfo.setVar(WallInfo::textureIndex, wallInfoIndex);
			wallInfo.setVar(
				WallInfo::itemDrop, wallInfoIndex 
					+ ItemInfo::defaultTileItems * 2
			);
		}

		Wall::Id wallId = static_cast<Wall::Id>(0);
		WallInfo* wallInfo = nullptr;

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewWall") {
				wallId = static_cast<Wall::Id>(std::stoi(value));
				wallInfo = &WallInfo::wallInfo[wallId];
			}
			else if (wallInfo != nullptr && !value.empty()) {
				WallInfo::Property property = WallInfo::End;
				
				for (int propertyIndex = 0; propertyIndex < WallInfo::End;
					propertyIndex++)
				{
					if (WallInfo::propertyStrings[propertyIndex] == attribute)
						property = static_cast<WallInfo::Property>(propertyIndex);
				}

				wallInfo->setVar(property, std::stof(value));
			}
		}
	}
}