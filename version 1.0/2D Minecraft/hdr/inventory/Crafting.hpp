#pragma once

// Dependencies
#include "Recipe.hpp"

namespace engine {
	// Crafting grid used when finding recipes. 
	extern ItemContainer validCraftingGrid[numOfCraftingGridSquares]; 

	void generateValidCraftingGrid(
		const ItemContainer* craftingGrid, bool fullTable
	);
	RecipeContainer getCraftingGridRecipe(const ItemContainer* craftingGrid, bool fullTable); 
}