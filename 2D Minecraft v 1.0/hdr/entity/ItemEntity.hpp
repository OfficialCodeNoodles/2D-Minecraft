#pragma once

// Dependencies
#include "Gravity.hpp"
#include "../inventory/Item.hpp"

namespace engine {
	class ItemEntity : public Entity {
	public:
		ItemContainer itemContainer; 
		int timeLeft; 

		ItemEntity(); 
		~ItemEntity(); 

		void update() override; 

		std::string getSaveString() const override; 

		static constexpr float maximumItemPickupDistance = 1.5f; 
		static const int defaultNumberOfTicksAlive; 

		static void dropItemEntity(
			ItemContainer itemContainer, gs::Vec2f position, 
			gs::Vec2f velocity = gs::Vec2f(-1, -1), bool preventPickup = false
		); 
	private:
	};
}