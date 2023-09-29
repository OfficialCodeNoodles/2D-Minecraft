#include "../../hdr/entity/ItemEntity.hpp"
#include "../../hdr/entity/Player.hpp"
#include "../../hdr/entity/Collision.hpp"
#include "../../hdr/graphics/UI.hpp"
#include "../../hdr/util/Random.hpp"

namespace engine {
	ItemEntity::ItemEntity() :
		Entity::Entity(),
		timeLeft(defaultNumberOfTicksAlive)
	{
		// Makes item spring in each direction. 
		velocity.x = (static_cast<float>(randomGenerator.generate() % 100) 
			/ 500.0f) - 0.1f;
		size = gs::Vec2f(0.5f, 0.5f); 
		mass = 0.75f; 
		itemContainer.count = 1; 
	}
	ItemEntity::~ItemEntity() {
	}

	void ItemEntity::update() {
		applyGravity(*this); 

		gs::util::approach(&velocity.x, 0.0f, 20.0f);
		if (collision::calculateEntityDistance(*player, *this) < 
				maximumItemPickupDistance
				&& !render::ui::gameOver)
			collision::attract(*player, *this); 

		collision::applyCollision(*this, *world); 
		updatePosition(); 

		dead = timeLeft <= 0; 
		timeLeft--; 

		if (itemContainer.count == 0)
			dead = true; 

		//std::cout << (int)itemContainer.item.id << "\n"; 
	}

	std::string ItemEntity::getSaveString() const {
		std::string saveString = Entity::getSaveString();

		const size_t symbolOffset = saveString.find_first_of('=');
		saveString.insert(
			symbolOffset + 1, " " + toString(
				static_cast<int>(Entity::Type::ItemEntity))
		);

		return saveString +
			"\nitemId = " + toString(itemContainer.item.id) +
			"\nitemTags = " + toString(itemContainer.item.tags.asInt) +
			"\nitemCount = " + toString(itemContainer.count) +
			"\ntimeLeft = " + toString(timeLeft); 
	}

	void ItemEntity::dropItemEntity(
		ItemContainer itemContainer, gs::Vec2f position, gs::Vec2f velocity,
		bool preventPickup)
	{
		if (itemContainer.item.isEmpty())
			return; 

		ItemEntity* itemEntity = new ItemEntity(); 

		itemEntity->position = position; 
		itemEntity->itemContainer = itemContainer; 

		if (velocity != gs::Vec2f(-1, -1))
			itemEntity->velocity = velocity; 
		if (preventPickup)
			itemEntity->timeLeft += 60; 

		addEntity(Type::ItemEntity, itemEntity);
	}

	const int ItemEntity::defaultNumberOfTicksAlive = 60 * 60 * 5;
}