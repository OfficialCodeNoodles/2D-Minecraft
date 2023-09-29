#pragma once

// Dependencies
#include "Item.hpp"
#include "../world/TileEntity.hpp"

namespace engine {
	class LootTable {
	public:
		struct LootPair {
			Item item; 
			gs::Vec2i amount; 
			float probability; 

			LootPair(); 
			LootPair(Item item, gs::Vec2i amount, float probability); 
			~LootPair() = default; 
		};
		enum Id {
			None,					ZombieDrops, 
			MelonDrops,				DeadBushDrops, 
			DungeonChestLoot,		GrassDrops,
			OakLeafDrops,			BirchLeafDrops,
			SpruceLeafDrops,		JungleLeafDrops, 
			WheatDrops,				CarrotDrops, 
			PotatoDrops,			SkeletonDrops,
			CreeperDrops,			RedstoneOreDrops,
			LapizOreDrops,			PigDrops,
			CowDrops 
		};

		LootTable(); 
		~LootTable() = default; 

		void addLootPair(const LootPair& lootPair); 
		void applyToChest(TileEntity* chest); 

		ItemContainer getLoot(); 

		static constexpr int numOfLootTables = 100; 
		static LootTable lootTables[numOfLootTables]; 
	private:
		std::vector<LootPair> lootPairs; 
		int lootIndex; 
	};

	void loadLootTables(); 
}