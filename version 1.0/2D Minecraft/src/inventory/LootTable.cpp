#include "../../hdr/inventory/LootTable.hpp"
#include "../../hdr/graphics/Render.hpp"
#include "../../hdr/util/Random.hpp"

namespace engine {
	LootTable::LootPair::LootPair() :
		amount({ 1, 1 }),
		probability(100.0f)
	{
	} 
	LootTable::LootPair::LootPair(Item item, gs::Vec2i amount, float probability) :
		item(item), 
		amount(amount), 
		probability(probability) 
	{
	}

	LootTable::LootTable() : lootIndex(0) {
	}

	void LootTable::addLootPair(const LootPair& lootPair) {
		lootPairs.push_back(lootPair); 
	}

	void LootTable::applyToChest(TileEntity* chest) {
		auto getEmptyInventorySlot = [chest](int slot) -> int {
			while (true) {
				if (chest->getItemCount(slot) == 0)
					return slot;

				slot++; 
				slot %= TileEntity::numOfChestSlots; 
			}
		}; 

		static Random randomGenerator; 

		std::vector<ItemContainer> loot; 
		int lootCount = 0; 
		ItemContainer generatedLoot = getLoot(); 

		while (!generatedLoot.item.isEmpty()) {
			loot.push_back(generatedLoot); 
			lootCount += loot.size(); 
			generatedLoot = getLoot(); 
		}

		// Stop if no loot was generated. 
		if (lootCount == 0)
			return;

		int chestSlotsRemaining = TileEntity::numOfChestSlots - loot.size(); 
		int itemsPerSlot = std::max(1, lootCount / chestSlotsRemaining); 

		randomGenerator.setSeed(0);

		for (auto& lootContainer : loot) {
			int lootRemaining = lootContainer.count; 
			
			for (int slotsLeft = lootRemaining / itemsPerSlot; slotsLeft > 0;
				slotsLeft--)
			{
				if (slotsLeft == 1)
					lootContainer.count = lootRemaining; 
				else 
					lootContainer.count = 1 
						+ (randomGenerator.generate() % lootRemaining); 
			
				lootRemaining -= lootContainer.count; 
			
				int slotSelected = randomGenerator.generate()
					% TileEntity::numOfChestSlots;
				slotSelected = getEmptyInventorySlot(slotSelected);
			
				chest->setItemContainer(slotSelected, lootContainer);
			
				if (lootRemaining <= 0)
					break; 
			}
		}
	}

	ItemContainer LootTable::getLoot() {
		LootPair* lootPair; 

		while (lootIndex < lootPairs.size()) {
			lootPair = &lootPairs[lootIndex]; 
			lootIndex++; 

			const float generatedValue = generateNormalizedFloat() * 100.0f; 

			if (generatedValue <= lootPair->probability) {
				const int lootPairAmountDelta = lootPair->amount.y 
					- lootPair->amount.x; 
				const int count = lootPairAmountDelta == 0 ? lootPair->amount.x
					: (randomGenerator.generate() % (lootPairAmountDelta + 1))
						+ lootPair->amount.x;

				return ItemContainer(lootPair->item, count); 
			}
		} 

		lootIndex = 0; 
		return ItemContainer(Item()); 
	}

	LootTable LootTable::lootTables[numOfLootTables];

	void loadLootTables() {
		const PairVector& pairs = loadPairedFile(
			render::assetDirectory + "data/lootTable.list"
		);

		LootTable::Id lootTableId = static_cast<LootTable::Id>(0);
		LootTable* lootTable = nullptr; 

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewTable") {
				lootTableId = static_cast<LootTable::Id>(std::stoi(value));
				lootTable = &LootTable::lootTables[lootTableId]; 
			}
			else if (lootTable != nullptr && !value.empty()) {
				if (attribute == "item") {
					LootTable::LootPair lootPair; 
					std::string number; 
					int commaIndex = 0; 

					for (char chr : value) {
						if (chr == ':') {
							switch (commaIndex) {
							case 0:
								lootPair.item = Item(
									static_cast<Item::Id>(std::stoi(number))
								); 
								break;
							case 1:
								lootPair.amount.x = std::stoi(number); 
								break; 
							case 2:
								lootPair.amount.y = std::stoi(number); 
								break; 
							case 3:
								lootPair.probability = std::stof(number);
								break; 
							}

							number = ""; 
							commaIndex++;
						}
						else
							number += chr; 
					}

					lootTable->addLootPair(lootPair); 
				}
			}
		}
	}
}