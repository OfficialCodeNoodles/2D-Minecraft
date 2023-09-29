#include "../../hdr/inventory/Recipe.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	Recipe::Recipe() :
		count(1),
		requiresFullTable(false)
	{
	}

	void Recipe::setItem(gs::Vec2i position, Item item) {
		items[getItemIndex(position)] = item;
	}

	Item Recipe::getItem(gs::Vec2i position) const {
		return items[getItemIndex(position)];
	}
	bool Recipe::isEmpty() const {
		for (auto& item : items) {
			// If any item isn't empty, then the recipe isn't either. 
			if (!item.isEmpty())
				return false;
		}

		return true;
	}

	Recipe Recipe::recipes[maxNumOfRecipes];

	int Recipe::getCraftingGridWidth(bool fullTable) {
		return fullTable ? 3 : 2;
	}
	int Recipe::getItemIndex(gs::Vec2i position, bool fullTable) {
		return (position.y * getCraftingGridWidth(fullTable)) + position.x;
	}

	FurnaceRecipe::FurnaceRecipe() {
	}

	FurnaceRecipe FurnaceRecipe::furnaceRecipes[maxNumOfFurnaceRecipes];

	RecipeContainer::RecipeContainer() : count(0) {
	}
	RecipeContainer::RecipeContainer(Recipe recipe) :
		recipe(recipe),
		count(0)
	{
	}
	RecipeContainer::RecipeContainer(int count) : count(count) {
	}
	RecipeContainer::RecipeContainer(Recipe recipe, int count) :
		recipe(recipe),
		count(count)
	{
	}

	FurnaceRecipeContainer::FurnaceRecipeContainer() : valid(false) {
	}
	FurnaceRecipeContainer::FurnaceRecipeContainer(FurnaceRecipe furnaceRecipe) :
		furnaceRecipe(furnaceRecipe), 
		valid(true) 
	{
	}

	FurnaceRecipeContainer getFurnaceRecipe(Item item) {
		FurnaceRecipeContainer furnaceRecipeContainer; 

		for (auto& furnaceRecipe : FurnaceRecipe::furnaceRecipes) {
			if (furnaceRecipe.input.id == item.id) {
				furnaceRecipeContainer.furnaceRecipe = furnaceRecipe; 
				furnaceRecipeContainer.valid = true; 
				break; 
			}
		}

		return furnaceRecipeContainer; 
	}
	void loadRecipes() {
		// Load crafting recipes. 
		{
			const PairVector& pairs = loadPairedFile(
				render::assetDirectory + "data/recipe.list"
			);

			int recipeId = 0;
			Recipe* recipe = nullptr;
			gs::Vec2i itemPosition = gs::Vec2i();

			for (auto& [attribute, value] : pairs) {
				if (attribute == "NewRecipe") {
					recipeId = std::stoi(value);
					recipe = &Recipe::recipes[recipeId];
					itemPosition.y = 0;
				}
				else if (recipe != nullptr && !value.empty()) {
					if (attribute == "items") {
						itemPosition.x = 0;

						StringPair pair("", value);

						// Keeps moving on the x-axis if item is found in recipe. 
						do {
							pair = seperate(pair.second, ' ');

							recipe->setItem(
								itemPosition, 
								Item(static_cast<Item::Id>(std::stoi(pair.first)))
							);
							itemPosition.x++;
						} while (!pair.second.empty());

						itemPosition.y++;

						// Checks if recipe size exceeds a 2x2 crafting grid. 
						if (std::max(itemPosition.x, itemPosition.y) == 3)
							recipe->requiresFullTable = true;
					}
					else if (attribute == "output")
						recipe->output = Item(static_cast<Item::Id>(std::stoi(value)));
					else if (attribute == "count")
						recipe->count = std::stoi(value);
				}
			}
		}
		// Load furnace recipes. 
		{
			const PairVector& pairs = loadPairedFile(
				render::assetDirectory + "data/furnaceRecipe.list"
			);

			int recipeId = 0;
			FurnaceRecipe* furnaceRecipe = nullptr;

			for (auto& [attribute, value] : pairs) {
				if (attribute == "NewRecipe") {
					recipeId = std::stoi(value);
					furnaceRecipe = &FurnaceRecipe::furnaceRecipes[recipeId];
				}
				else if (furnaceRecipe != nullptr && !value.empty()) {
					if (attribute == "input")
						furnaceRecipe->input.id = static_cast<Item::Id>(std::stoi(value));
					else if (attribute == "output")
						furnaceRecipe->output.id = static_cast<Item::Id>(std::stoi(value)); 
				}
			}
		}
	}
}