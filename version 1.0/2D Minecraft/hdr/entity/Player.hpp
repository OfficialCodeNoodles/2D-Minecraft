#pragma once

// Dependencies
#include "Gravity.hpp"
#include "../inventory/Item.hpp"
#include "../util/Input.hpp"

namespace engine {
	class Player : public Entity {
	public:
		static constexpr int numOfInventorySlots = 36;

		ItemContainer inventory[numOfInventorySlots]; 
		int health;
		int foodLevel; 
		bool crouching; 
		float miningDuration; 
		bool interactionLocked; 
		bool placeOnGround; 

		Player(); 
		~Player(); 

		void update() override; 
		void hurt(int damage, bool playSound = true); 

		ItemContainer& getItemContainerSelected(); 
		std::string getSaveString() const override; 

		static const int maxHealth; 
		static const int maxFoodLevel; 
		static const int foodEatingCooldownDuration; 
		static const int numOfHotbarSlots;
		static const float maxReachDistance; 
		static const int tilePlacementCooldownDuration; 
	private:
		float foodSaturationLevel; 
		float foodExhaustionLevel; 
		int foodTickTimer; 
		int burningTicks; 
		gs::Vec2i crouchingTile; 

		void resetTileFocus(); 
		void runFreeCamera(); 
	};

	extern Player* player; 
	extern gs::Vec2f mouseTileLocation; 
	extern gs::Vec2i mouseTilePosition; 
	extern bool blockInteraction; 
	extern gs::Vec2i tileBeingBroken; 
	extern int tileBreakDuration; 
	extern float tileBreakPercentage; 
	extern int tilePlacementCooldown; 
	extern gs::Vec2i tileEntityPosition; 
	extern int foodEatingCooldown; 

	void createPlayer(); 
}