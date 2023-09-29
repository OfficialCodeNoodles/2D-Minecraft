#include "../../hdr/inventory/Item.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	Item::Tags::Tags() : asInt(0ull) {
	}

	Item::Item() : id(Empty) {
	}
	Item::Item(Item::Id itemId) : id(itemId) {
		init(); 
	}
	Item::Item(TagInt tags) : id(Empty) {
		this->tags.asInt = tags; 
	}
	Item::Item(Item::Id itemId, TagInt tags) : id(itemId) {
		this->tags.asInt = tags; 
	}

	void Item::init() {
		tags.durability = getVar(ItemInfo::maxDurability);
	}

	const std::string& Item::getName() const {
		return ItemInfo::itemInfo[id].name; 
	}
	float Item::getVar(PropertyInt propertyInt) const {
		return ItemInfo::getVar(
			id, static_cast<ItemInfo::Property>(propertyInt)
		);
	}

	bool Item::isEmpty() const {
		return id == 0; 
	}

	bool Item::compare(Item item1, Item item2) {
		return item1.id == item2.id; 
	}

	ItemContainer::ItemContainer() : item(Item()), count(0) {
	}
	ItemContainer::ItemContainer(Item item) : 
		item(item), 
		count(0) 
	{
	}
	ItemContainer::ItemContainer(int count) : 
		item(Item()), 
		count(count) 
	{
	}
	ItemContainer::ItemContainer(Item item, int count) :
		item(item),
		count(count)
	{
	}

	void ItemContainer::update() {
		if (count <= 0)
			item = Item(); 
	}

	ItemInfo::ItemInfo() {
		setVar(type, static_cast<float>(PlacementType::None)); 
		setVar(stackSize, defaultStackSize); 
		setVar(itemGroup, 0); 
		setVar(toolType, static_cast<float>(ToolType::None)); 
		setVar(toolStrength, defaultToolStrength); 
		setVar(toolMineSpeed, defaultToolMineSpeed); 
		setVar(maxDurability, 0); 
		setVar(fuelValue, 0); 
		setVar(damage, 1); 
		setVar(foodPoints, 0);
		setVar(foodSaturation, 5.0f);
		setVar(angleOffset, 0.0f); 
	}

	void ItemInfo::setVar(Property property, float value) {
		vars[property] = value; 
	}

	float ItemInfo::getVar(Property property) const {
		return vars[property]; 
	}

	const int ItemInfo::defaultTileItems = 1000;
	const int ItemInfo::defaultStackSize = 64;
	const float ItemInfo::defaultToolStrength = 0.0f;
	const float ItemInfo::defaultToolMineSpeed = 1.0f;
	const std::string ItemInfo::propertyStrings[ItemInfo::End] = {
		"textureIndex",				"type", 
		"tileIndex",				"animationAngle", 
		"stackSize",				"itemGroup", 
		"toolType",					"toolStrength", 
		"toolMineSpeed",			"maxDurability",
		"fuelValue",				"damage",
		"foodPoints",				"foodSaturation",
		"angleOffset"
	}; 
	ItemInfo ItemInfo::itemInfo[ItemInfo::numOfItems];

	float ItemInfo::getVar(Item::Id itemId, Property property) {
		return itemInfo[itemId].getVar(property); 
	}

	void loadItemInfo() {
		const PairVector& pairs = loadPairedFile(
			render::assetDirectory + "data/item.list"
		);

		for (int itemInfoIndex = 0; itemInfoIndex < ItemInfo::numOfItems;
			itemInfoIndex++)
		{
			ItemInfo& itemInfo = ItemInfo::itemInfo[itemInfoIndex];
			int textureIndex = itemInfoIndex; 

			if (itemInfoIndex < ItemInfo::defaultTileItems) {
			}
			// Generates all of the default blocks. 
			else if (itemInfoIndex < ItemInfo::defaultTileItems * 2) {
				itemInfo.setVar(
					ItemInfo::type, 
					static_cast<float>(ItemInfo::PlacementType::Block)
				);
				itemInfo.setVar(
					ItemInfo::tileIndex, itemInfoIndex 
						% ItemInfo::defaultTileItems
				); 
			}
			// Generates all of the default walls. 
			else {
				itemInfo.setVar(
					ItemInfo::type, 
					static_cast<float>(ItemInfo::PlacementType::Wall)
				);
				itemInfo.setVar(
					ItemInfo::tileIndex, itemInfoIndex 
						% ItemInfo::defaultTileItems
				);
			}

			itemInfo.setVar(ItemInfo::textureIndex, textureIndex); 
		}

		Item::Id itemId = static_cast<Item::Id>(0);
		ItemInfo* itemInfo = nullptr;

		for (auto& [attribute, value] : pairs) {
			if (attribute == "NewItem") {
				itemId = static_cast<Item::Id>(std::stoi(value));
				itemInfo = &ItemInfo::itemInfo[itemId];
			}
			else if (itemInfo != nullptr && !value.empty()) {
				if (attribute == "name") {
					itemInfo->name = value; 
					continue; 
				}
				
				ItemInfo::Property property = ItemInfo::End;

				for (int propertyIndex = 0; propertyIndex < ItemInfo::End;
					propertyIndex++)
				{
					if (ItemInfo::propertyStrings[propertyIndex] == attribute)
						property = static_cast<ItemInfo::Property>(propertyIndex);
				}

				itemInfo->setVar(property, std::stof(value));
			}
		}
	}
	ItemContainer addItemToInventory(
		ItemContainer itemContainer, ItemContainer* itemContainers, int slots)
	{
		const ItemInfo& itemInfo = ItemInfo::itemInfo[itemContainer.item.id]; 
		
		int emptyItemContainerIndex = -1;
		bool noRemainingMatch = false; 

		for (int slotIndex = 0; slotIndex < slots && itemContainer.count > 0; 
			slotIndex++) 
		{
			ItemContainer& currentItemContainer = itemContainers[slotIndex]; 
			const ItemInfo& currentItemInfo = 
				ItemInfo::itemInfo[currentItemContainer.item.id]; 

			if (noRemainingMatch && currentItemContainer.count == 0) {
				emptyItemContainerIndex = slotIndex; 
				break; 
			}

			if (Item::compare(itemContainer.item, currentItemContainer.item)
				&& currentItemContainer.count != 0) 
			{
				const int maxItemsToAdd = itemInfo.getVar(ItemInfo::stackSize)
					- currentItemContainer.count; 
				const int itemsToAdd = std::min(maxItemsToAdd, 
					itemContainer.count);

				currentItemContainer.count += itemsToAdd; 
				itemContainer.count -= itemsToAdd; 
			}

			if (slotIndex == slots - 1 && itemContainer.count > 0) {
				if (!noRemainingMatch) {
					slotIndex = -1; 
					noRemainingMatch = true; 
				}
			}
		}

		if (emptyItemContainerIndex != -1) {
			itemContainers[emptyItemContainerIndex] = itemContainer;
			itemContainer.count = 0; 
		}

		return itemContainer; 
	}
}