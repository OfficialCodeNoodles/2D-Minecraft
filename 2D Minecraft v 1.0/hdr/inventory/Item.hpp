#pragma once

// Depencendies
#include "../util/Loader.hpp"
#include "../world/Tile.hpp"

namespace engine {
	class Item {
	public:
		struct Tags {
			using Byte = unsigned char;
			using Word = unsigned short;

			union {
				struct {
					Word durability : 11; 
				};
				TagInt asInt;
			};

			Tags();
			~Tags() = default;
		} tags;
		enum Id {
			Empty,					WoodenSword, 
			StoneSword,				IronSword,
			GoldSword,				DiamondSword, 
			WoodenPickaxe,			StonePickaxe, 
			IronPickaxe,			GoldPickaxe, 
			DiamondPickaxe,			WoodenAxe, 
			StoneAxe,				IronAxe, 
			GoldAxe,				DiamondAxe, 
			WoodenShovel,			StoneShovel,
			IronShovel,				GoldShovel,
			DiamondShovel,			WoodenHoe, 
			StoneHoe,				IronHoe, 
			GoldHoe,				DiamondHoe, 
			Coal,					RawCopper, 
			RawIron,				RawGold, 
			Redstone,				Lapiz, 
			Emerald,				Diamond, 
			Stick,					Torch,
			Bamboo,					IronIngot, 
			GoldIngot,				MelonSlice,
			RottenFlesh,			Bucket,
			WaterBucket,			LavaBucket,
			WheatSeeds,				Wheat,
			Bread,					Apple,
			Carrot,					Potato,
			BakedPotato,			Bow,
			Bone = 55,				BoneMeal, 
			GunPowder,				Porkchop, 
			CookedPorkchop,			Beef,
			Steak 
		} id;
		
		Item(); 
		Item(Item::Id itemId); 
		Item(TagInt tags); 
		Item(Item::Id itemId, TagInt tags); 
		~Item() = default; 

		void init(); 

		const std::string& getName() const; 
		float getVar(PropertyInt propertyInt) const;

		bool isEmpty() const; 

		static bool compare(Item item1, Item item2); 
	private:
	};

	struct ItemContainer {
		Item item; 
		int count; 

		ItemContainer();
		ItemContainer(Item item); 
		ItemContainer(int count); 
		ItemContainer(Item item, int count); 
		~ItemContainer() = default; 

		void update(); 
	};

	class ItemInfo {
	public:
		enum class PlacementType { None, Block, Wall, };
		enum class ToolType { None, Pickaxe, Axe, Shovel, Sword, Hoe };
		enum Property {
			textureIndex,			type, 
			tileIndex,				animationAngle, 
			stackSize,				itemGroup, 
			toolType,				toolStrength, 
			toolMineSpeed,			maxDurability, 
			fuelValue,				damage, 
			foodPoints,				foodSaturation,
			angleOffset, 
			End
		};
		std::string name; 

		ItemInfo(); 
		~ItemInfo() = default; 

		void setVar(Property property, float value); 

		float getVar(Property property) const; 

		static constexpr int numOfItems = 3000; 
		static const int defaultTileItems; 
		static const int defaultStackSize; 
		static const float defaultToolStrength;
		static const float defaultToolMineSpeed; 
		static const std::string propertyStrings[End]; 
		static ItemInfo itemInfo[numOfItems]; 

		static float getVar(Item::Id itemId, Property property); 
	private:
		float vars[End]; 
	};

	void loadItemInfo(); 
	ItemContainer addItemToInventory(
		ItemContainer itemContainer, ItemContainer* itemContainers, int slots
	);
}