#pragma once

// Dependencies
#include "Item.hpp"

namespace engine {
	constexpr int numOfCraftingGridSquares = 9;

	class Recipe {
	public:
		Item output;
		int count;
		bool requiresFullTable;

		Recipe();
		~Recipe() = default;

		void setItem(gs::Vec2i position, Item item);

		Item getItem(gs::Vec2i position) const;
		bool isEmpty() const;

		static constexpr int maxNumOfRecipes = 100;
		static Recipe recipes[maxNumOfRecipes];

		static int getCraftingGridWidth(bool fullTable);
		static int getItemIndex(gs::Vec2i position, bool fullTable = true);
	private:
		Item items[numOfCraftingGridSquares];
	};

	class FurnaceRecipe {
	public:
		Item input, output; 

		FurnaceRecipe(); 
		~FurnaceRecipe() = default; 

		static constexpr int maxNumOfFurnaceRecipes = 100; 
		static FurnaceRecipe furnaceRecipes[maxNumOfFurnaceRecipes]; 
	private:
	};

	// Simple container to keep track of a crafting recipe, as well as the 
	// amount of items that can be crafted from it. 
	struct RecipeContainer {
		Recipe recipe;
		int count;

		RecipeContainer();
		RecipeContainer(Recipe recipe);
		RecipeContainer(int count);
		RecipeContainer(Recipe recipe, int count);
		~RecipeContainer() = default;
	};

	struct FurnaceRecipeContainer {
		FurnaceRecipe furnaceRecipe; 
		bool valid; 

		FurnaceRecipeContainer(); 
		FurnaceRecipeContainer(FurnaceRecipe furnaceRecipe); 
		~FurnaceRecipeContainer() = default; 
	};

	FurnaceRecipeContainer getFurnaceRecipe(Item item); 
	void loadRecipes();
}