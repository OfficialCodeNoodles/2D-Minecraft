#pragma once

// Dependencies
#include "Tile.hpp"
#include "../inventory/Item.hpp"
#include "../inventory/Recipe.hpp"

namespace engine {
	class TileEntity {
	public:
		struct Tags {
			using Byte = unsigned char;

			union {
				struct {
					Byte smeltingProgress : 8; 
					Byte fuelProgress : 5; 
					Byte fuelRemaining : 8;
					Byte fuelMax : 8;
				};
				TagInt asInt;
			};

			Tags();
			~Tags() = default;
		} tags;
		static_assert(
			sizeof(Tags) <= sizeof(TagInt),
			"Too much data carryed in Tile-Entity Tags"
		);

		gs::Vec2i position; 

		TileEntity(); 
		TileEntity(gs::Vec2i position); 
		TileEntity(TagInt tags); 
		TileEntity(gs::Vec2i position, TagInt tags); 
		~TileEntity() = default; 

		void update(Block& block); 
		void clearItemSlot(int itemSlot); 
		void clear(); 

		void setItemId(int itemSlot, Item::Id itemId); 
		void setItemTags(int itemSlot, TagInt tags); 
		void setItemCount(int itemSlot, int count); 
		void setItem(int itemSlot, Item item); 
		void setItemContainer(int itemSlot, ItemContainer itemContainer); 

		Item::Id getItemId(int itemSlot) const; 
		TagInt getItemTags(int itemSlot) const;
		int getItemCount(int itemSlot) const; 
		Item getItem(int itemSlot) const; 
		ItemContainer& getItemContainer(int itemSlot); 
		std::string getSaveString() const; 

		static constexpr int maxNumOfItemSlots = 27; 
		static constexpr int numOfChestSlots = 27; 
		static const int smeltingDuration; 
		static const int fuelDuration; 

		static int getItemContainerSize(const Block& block); 
	private:
		ItemContainer itemContainers[maxNumOfItemSlots]; 
	};
}