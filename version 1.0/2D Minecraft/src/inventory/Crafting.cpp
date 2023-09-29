#include "../../hdr/inventory/Crafting.hpp"

namespace engine {
	ItemContainer validCraftingGrid[numOfCraftingGridSquares];

	void generateValidCraftingGrid(
		const ItemContainer* craftingGrid, bool fullTable) 
	{
		const int craftingGridWidth = Recipe::getCraftingGridWidth(fullTable); 

		// Row and column checkers. 

		auto hasEmptyBaseRow = [&]() -> bool {
			for (int xpos = 0; xpos < craftingGridWidth; xpos++) {
				const int index = Recipe::getItemIndex({ xpos, 0 }, fullTable);

				if (validCraftingGrid[index].count > 0)
					return false; 
			}

			return true; 
		}; 
		auto hasEmptyBaseColumn = [&]() -> bool {
			for (int ypos = 0; ypos < craftingGridWidth; ypos++) {
				const int index = Recipe::getItemIndex({ 0, ypos }, fullTable);

				if (validCraftingGrid[index].count > 0)
					return false;
			}

			return true;
		};

		// Clears valid crafting grid. 
		for (auto& itemContainer : validCraftingGrid)
			itemContainer = ItemContainer(Item()); 

		// Copys items from crafting grid over to valid crafting grid, and then
		// flips them due to them being upside down. 
		for (int itemContainerIndex = 0, reversedIndex = fullTable ? 6 : 2; 
			itemContainerIndex < craftingGridWidth * craftingGridWidth; 
			itemContainerIndex++, reversedIndex++)
		{
			const ItemContainer& itemContainer =
				craftingGrid[itemContainerIndex];
			
			// Resets reversedIndex every row completed. 
			if (itemContainerIndex > 0 && itemContainerIndex % craftingGridWidth == 0)
				reversedIndex -= fullTable ? 6 : 4;

			validCraftingGrid[reversedIndex] = itemContainer.count > 0
				? itemContainer : ItemContainer(Item());
		}

		gs::Vec2i itemPosition = gs::Vec2i(); 
		int rowsPushed = 0; 
		int columnsPushed = 0;

		// Shifts grid up while it still has an empty base row.
		while (hasEmptyBaseRow() && rowsPushed < craftingGridWidth) {
			for (itemPosition.y = 1; itemPosition.y < craftingGridWidth + 1;
				itemPosition.y++)
			{
				for (itemPosition.x = 0; itemPosition.x < craftingGridWidth;
					itemPosition.x++)
				{
					// Actual position being overwritten. 
					const gs::Vec2i realItemPosition = itemPosition 
						// Gets shifted up. 
						- gs::Vec2i(0, 1); 

					// Resets container if next position is invalid. 
					if (itemPosition.y >= craftingGridWidth) {
						validCraftingGrid[Recipe::getItemIndex(realItemPosition, 
							fullTable)] = ItemContainer(Item()); 
						continue; 
					}

					validCraftingGrid[Recipe::getItemIndex(realItemPosition, 
						fullTable)] = validCraftingGrid[Recipe::getItemIndex(
							itemPosition, fullTable)]; 
				}
			}

			rowsPushed++; 
		}

		// Shifts grid left while it still has an empty base column.
		while (hasEmptyBaseColumn() && columnsPushed < craftingGridWidth) {
			for (itemPosition.x = 1; itemPosition.x < craftingGridWidth + 1;
				itemPosition.x++)
			{
				for (itemPosition.y = 0; itemPosition.y < craftingGridWidth;
					itemPosition.y++)
				{
					// Actual position being overwritten. 
					const gs::Vec2i realItemPosition = itemPosition
						// Gets shifted left. 
						- gs::Vec2i(1, 0);

					// Resets container if next position is invalid. 
					if (itemPosition.x >= craftingGridWidth) {
						validCraftingGrid[Recipe::getItemIndex(realItemPosition,
							fullTable)] = ItemContainer(Item());
						continue; 
					}

					validCraftingGrid[Recipe::getItemIndex(realItemPosition, 
						fullTable)] = validCraftingGrid[Recipe::getItemIndex(
							itemPosition, fullTable)];
				}
			}

			columnsPushed++;
		}
	}
	RecipeContainer getCraftingGridRecipe(const ItemContainer* craftingGrid, bool fullTable) {
		generateValidCraftingGrid(craftingGrid, fullTable);

		const int craftingGridWidth = Recipe::getCraftingGridWidth(fullTable);

		RecipeContainer recipeContainer; 
		gs::Vec2i itemPosition = gs::Vec2i();
		// Keeps track of the lowest item count in the recipe. This is used to
		// see how many times an item can be crafted. 
		int lowestItemCount = 64; 

		for (auto& recipe : Recipe::recipes) {
			// Ensures recipe isn't empty. 
			if (recipe.isEmpty())
				continue;

			bool recipeMatch = true;

			// Doesn't allow 2x2 crafting grid to craft 3x3 recipes. 
			if (recipe.requiresFullTable && !fullTable)
				continue; 

			for (itemPosition.x = 0; itemPosition.x < craftingGridWidth; 
				itemPosition.x++) 
			{
				for (itemPosition.y = 0; itemPosition.y < craftingGridWidth;
					itemPosition.y++)
				{
					const int itemIndex = Recipe::getItemIndex(
						itemPosition, fullTable);
					const ItemContainer& validCraftingContainer =
						validCraftingGrid[itemIndex];
					const Item item = recipe.getItem(itemPosition); 
					const bool useItemGroup = item.id < 0; 
					const int craftingContainerItemGroup = ItemInfo::itemInfo[
						validCraftingContainer.item.id
					].getVar(ItemInfo::itemGroup); 

					// Invalid if recipe has an empty spot and the 
					// crafting grid has item/s. 
					if (item.isEmpty()) {
						if (validCraftingContainer.count > 0)
							recipeMatch = false;
					}
					else if (validCraftingContainer.count > 0) {
						// Invalid if item in crafting grid doesn't match the
						// recipe. 
						if (!useItemGroup && validCraftingContainer.item.id != item.id)
							recipeMatch = false;
						else if (useItemGroup && craftingContainerItemGroup != -item.id)
							recipeMatch = false; 
						else 
							lowestItemCount = std::min(lowestItemCount, 
								validCraftingContainer.count); 
					}
					// Invalid if crafting grid is missing a needed item. 
					else
						recipeMatch = false; 

					if (!recipeMatch)
						goto LoopReset; 
				}
			}

			if (recipeMatch) {
				recipeContainer = recipe; 
				recipeContainer.count = lowestItemCount * recipe.count; 
				break; 
			}
LoopReset:;
		}

		return recipeContainer; 
	}
}