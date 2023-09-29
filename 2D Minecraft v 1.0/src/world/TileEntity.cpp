#include "../../hdr/world/TileEntity.hpp"
#include "../../hdr/graphics/Window.hpp"
#include "../../hdr/inventory/LootTable.hpp"

namespace engine {
	TileEntity::Tags::Tags() : asInt(0ull){
	} 
	
	TileEntity::TileEntity() {
	}
	TileEntity::TileEntity(gs::Vec2i position) : position(position) {
	}
	TileEntity::TileEntity(TagInt tags) {
		this->tags.asInt = tags;
	}
	TileEntity::TileEntity(gs::Vec2i position, TagInt tags) : 
		position(position) 
	{
		this->tags.asInt = tags; 
	}
	
	void TileEntity::update(Block& block) {
		for (auto& itemContainer : itemContainers)
			itemContainer.update(); 

		switch (block.id) {
		case Block::Furnace:
		{
			ItemContainer& smeltingContainer = getItemContainer(0);
			ItemContainer& fuelContainer = getItemContainer(1);
			ItemContainer& outputContainer = getItemContainer(2);
			// Container used to keep track of item is being currently smelted. 
			ItemContainer& smeltingReferenceContainer = getItemContainer(3);

			if (render::window::ticks % 3 == 0) {
				const FurnaceRecipeContainer furnaceRecipeContainer =
					getFurnaceRecipe(smeltingContainer.item);
				const int fuelValue = fuelContainer.item.getVar(ItemInfo::fuelValue);

				// Starts smelting process. 
				if (outputContainer.count >= furnaceRecipeContainer.
						furnaceRecipe.output.getVar(ItemInfo::stackSize))
					smeltingReferenceContainer.item = Item();
				else if (tags.smeltingProgress == 0 && fuelValue > 0
					&& furnaceRecipeContainer.valid
					&& ((outputContainer.item.id == furnaceRecipeContainer.
						furnaceRecipe.output.id) || outputContainer.count == 0))
				{
					smeltingReferenceContainer.item = furnaceRecipeContainer.
						furnaceRecipe.input;
					smeltingReferenceContainer.count = 1; 
				}
				// Handle item being fully smelted. 
				else if (tags.smeltingProgress >= smeltingDuration) {
					tags.smeltingProgress = 0;
					// Removes one input item.
					smeltingContainer.count--;

					outputContainer.item.id = furnaceRecipeContainer.
						furnaceRecipe.output.id;
					// Adds one output item. 
					outputContainer.count++;
				}

				// Handle remaining fuel. 
				if (tags.fuelRemaining > 0) {
					tags.fuelProgress++; 

					// Removes one fuel value once the duration reaches the 
					// fuel duration. 
					if (tags.fuelProgress % fuelDuration == 0 
						&& tags.fuelProgress > 0)
					{
						tags.fuelRemaining--;
						tags.fuelProgress = 0; 
					}

					// Use lit furnace animation when fuel remains in furnace. 
					block.tags.animationOffset = 1;
				}
				else
					// Use unlit furnace animation when no fuel remains in 
					// furnace. 
					block.tags.animationOffset = 0; 

				// Continues smelting progress as long as the item being
				// smelted matches the reference.
				if (smeltingContainer.item.id == smeltingReferenceContainer.item.id
					&& !smeltingReferenceContainer.item.isEmpty())
				{
					// Handles empty fuel item. 
					if (tags.fuelRemaining == 0) {
						if (smeltingContainer.count > 0) {
							// Continues smelting if fuel remains. 
							if (fuelContainer.count > 0) {
								tags.fuelRemaining = fuelValue;
								tags.fuelMax = fuelValue;
							}
							else
								tags.smeltingProgress = 0;

							fuelContainer.count--;
						}
					}
					// Increments smelting progress. 
					else if (tags.fuelRemaining > 0) 
						tags.smeltingProgress++;
				}
				else 
					tags.smeltingProgress = 0;
			}
		}
			break; 
		case Block::Chest:
			if (block.tags.lootTable != LootTable::None) {
				LootTable::lootTables[block.tags.lootTable].applyToChest(this); 
				block.tags.lootTable = LootTable::None; 
			}

			break; 
		}
	}
	void TileEntity::clearItemSlot(int itemSlot) {
		itemContainers[itemSlot] = ItemContainer(Item()); 
	}
	void TileEntity::clear() {
		for (int itemSlot = 0; itemSlot < maxNumOfItemSlots; itemSlot++)
			clearItemSlot(itemSlot); 
	}

	void TileEntity::setItemId(int itemSlot, Item::Id itemId) {
		itemContainers[itemSlot].item.id = itemId; 
	}
	void TileEntity::setItemTags(int itemSlot, TagInt tags) {
		itemContainers[itemSlot].item.tags.asInt = tags; 
	}
	void TileEntity::setItemCount(int itemSlot, int count) {
		itemContainers[itemSlot].count = count; 
	}
	void TileEntity::setItem(int itemSlot, Item item) {
		itemContainers[itemSlot].item = item; 
	}
	void TileEntity::setItemContainer(
		int itemSlot, ItemContainer itemContainer) 
	{
		itemContainers[itemSlot] = itemContainer; 
	}

	Item::Id TileEntity::getItemId(int itemSlot) const {
		return itemContainers[itemSlot].item.id; 
	}
	TagInt TileEntity::getItemTags(int itemSlot) const {
		return itemContainers[itemSlot].item.tags.asInt; 
	}
	int TileEntity::getItemCount(int itemSlot) const {
		return itemContainers[itemSlot].count;
	}
	Item TileEntity::getItem(int itemSlot) const {
		return itemContainers[itemSlot].item;
	}
	ItemContainer& TileEntity::getItemContainer(int itemSlot) {
		return itemContainers[itemSlot]; 
	}
	std::string TileEntity::getSaveString() const {
		std::string saveString = "New Tile Entity"
			"\nxpos = " + toString(position.x) +
			"\nypos = " + toString(position.y) +
			"\ntags = " + toString(tags.asInt) + 
			"\nitems = ";

		// Add items to save string. 
		for (int itemSlot = 0; itemSlot < TileEntity::maxNumOfItemSlots;
			itemSlot++)
		{
			const Item item = getItem(itemSlot);
			saveString += toString(item.id) + ":" + toString(item.tags.asInt) 
				+ ":" + toString(getItemCount(itemSlot)) + " ";
		}

		return saveString; 
	}

	int TileEntity::getItemContainerSize(const Block& block) {
		switch (block.id) {
		case Block::Furnace: return 3;
		}

		return maxNumOfItemSlots;
	}

	const int TileEntity::smeltingDuration = 159;
	const int TileEntity::fuelDuration = 20;
}