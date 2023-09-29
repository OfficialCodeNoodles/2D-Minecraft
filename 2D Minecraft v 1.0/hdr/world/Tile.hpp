#pragma once

// Dependencies
#include "../util/Loader.hpp"

namespace engine {
	using TagInt = unsigned long long; 
	using PropertyInt = int; 

	struct Block {
		struct Tags {
			using Byte = unsigned char; 
			using Word = unsigned short; 

			union {
				struct {
					bool ignoreCollision : 1;
					bool persistance : 1; 
					bool isFluidSource : 1;
					bool naturalBlock : 1; 
					Byte animationOffset : 3;
					Byte rotation : 2;
					Byte fluidLevel : 3; 
					Byte lootTable : 4; 
				};
				TagInt asInt; 
			}; 

			Tags(); 
			~Tags() = default; 
		} tags;
		static_assert(
			sizeof(Tags) <= sizeof(TagInt),
			"Too much data carryed in Block Tags"
		); 
		enum Id : short {
			Invalid = -1, 
			Air,					Dirt, 
			GrassBlock,				Stone, 
			Cobblestone,			Sand, 
			Sandstone,				Gravel, 
			Andesite,				Granite,
			Diorite,				Deepslate, 
			CobbledDeepslate,		OakLog, 
			OakLeaves,				Bedrock = 16, 
			BirchLog,				BirchLeaves, 
			Cactus = 20,			CoalOre, 
			DeepslateCoalOre,		CopperOre, 
			DeepslateCopperOre,		IronOre, 
			DeepslateIronOre,		GoldOre, 
			DeepslateGoldOre,		RedstoneOre, 
			DeepslateRedstoneOre,	LapizOre, 
			DeepslateLapizOre,		EmeraldOre, 
			DeepslateEmeraldOre,	DiamondOre, 
			DeepslateDiamondOre,	Grass, 
			Torch,					CraftingTable = 42, 
			Water = 44,				SpruceLog, 
			SpruceLeaves,			Fern = 48, 
			Poppy,					Dandelion, 
			OakPlanks,				BirchPlanks, 
			SprucePlanks,			Furnace,
			Chest = 56,				DeadBush = 58, 
			ClayBlock,				JungleLog, 
			JungleLeaves,			JunglePlanks = 63, 
			MelonBlock,				BambooStalk,
			MossyCobblestone = 70,	Spawner = 71,
			Lava = 72,				OakSapling,
			BirchSapling,			SpruceSapling,
			JungleSapling,			BrownMushroom, 
			RedMushroom,			BlueOrchid, 
			FarmLand,				Wheat = 82,	
			AdultWheat = 89,		Carrot = 90,			
			AdultCarrot = 93,		Potato = 94,
			AdultPotato = 97,		AcaciaLog, 
			AcaciaLeaves,			AcaciaPlanks = 101, 
			AcaciaSapling, 
			End
		} id;
		enum class UpdateState : Tags::Byte { 
			NoUpdate, NeedsUpdate, UpdateNext
		} updateState; 

		Block();
		Block(Block::Id blockId);
		Block(TagInt tags);
		Block(Block::Id blockId, TagInt tags);
		~Block() = default;

		void init(); 

		float getVar(PropertyInt propertyInt) const;

		bool isEmpty() const; 
		bool isFluid() const; 
		bool isFluidBreakable() const; 
		bool isSolid() const; 
	};

	struct Wall {
		struct Tags {
			using Byte = unsigned char;

			union {
				struct {
				};
				TagInt asInt;
			};

			Tags();
			~Tags() = default;
		} tags;
		static_assert(
			sizeof(Tags) <= sizeof(TagInt),
			"Too much data carryed in Wall Tags"
		);
		enum Id : short {
			Air,					Dirt, 
			Stone,					Deepslate, 
			Sand,					Cobblestone,			
			MossyCobblestone, 
			End
		} id;

		Wall(); 
		Wall(Wall::Id wallId); 
		Wall(TagInt tags); 
		Wall(Wall::Id wallId, TagInt tags); 
		~Wall() = default; 

		void init(); 

		float getVar(PropertyInt propertyInt) const;

		bool isEmpty() const; 
	};

	class BlockInfo {
	public:
		enum class BlockUpdate {
			None,					FallingBlock, 
			Leaves,					Torch, 
			Fluid,					Grass,
			Bamboo,					Sapling,
			Crop,					FarmLand
		};
		enum class BlockDependencyType {
			None,					Torch,
			GrassPlant,				SandPlant,
			Cactus,					Bamboo,
			Crop
		};
		enum Property : PropertyInt {
			textureIndex,			lightIndex, 
			requiresTileEntity,		collisionType,
			blockUpdate,			blockDependencyType, 
			randomRotation,			hasTransparency,
			hasBlockOverlay,		renderUnderside, 
			generationReplacable,	foreground, 
			requiredToolToBreak,	requiredToolStrength,
			hardness,				itemDrop, 
			dropItemWithIncorrectTool, inventoryMenu, 
			horizontalShift,		applyAnimationLighting, 
			lootTable,				soundEvent,
			fluidBreakable, 
			End
		};

		BlockInfo(); 
		~BlockInfo() = default; 

		void setVar(Property property, float value);

		float getVar(Property property) const; 

		static constexpr int numOfBlocks = 255; 
		static const std::string propertyStrings[End]; 
		static BlockInfo blockInfo[BlockInfo::numOfBlocks]; 

		static float getVar(Block::Id blockId, Property property); 
	private:
		float vars[End]; 
	};

	class WallInfo {
	public:
		enum Property : PropertyInt {
			textureIndex,			lightIndex,
			hasTransparency,		
			requiredToolToBreak,	requiredToolStrength,
			hardness,				itemDrop,
			dropItemWithIncorrectTool, soundEvent, 
			End
		};

		WallInfo();
		~WallInfo() = default; 

		void setVar(Property property, float value); 

		float getVar(Property property) const; 

		static constexpr int numOfWalls = 100; 
		static const std::string propertyStrings[End];  
		static WallInfo wallInfo[WallInfo::numOfWalls]; 

		static float getVar(Wall::Id wallId, Property property); 
	private: 
		float vars[End]; 
	};

	void loadBlockInfo(); 
	void loadWallInfo(); 
}