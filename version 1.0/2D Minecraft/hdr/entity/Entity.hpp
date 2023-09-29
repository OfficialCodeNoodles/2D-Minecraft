#pragma once

// Dependencies
#include "Model.hpp"
#include "Collision.hpp"

namespace engine {
	using collision::Collisions; 

	class Entity {
	public:
		ModelTransform modelTransform; 
		Collisions collisions; 
		gs::Vec2f position; 
		gs::Vec2f velocity; 
		gs::Vec2f velocityScaler; 
		gs::Vec2f size; 
		float mass; 
		int immunityFrames; 
		bool dead; 

		enum class Type { None, PlayerEntity, Mob, ItemEntity, ProjectileEntity };

		Entity(); 
		virtual ~Entity(); 

		virtual void update(); 

		virtual std::string getSaveString() const; 

		static int numOfEntities; 
	protected:
		void updatePosition(); 
	};

	using EntityPair = std::pair<Entity::Type, Entity*>; 

	extern std::vector<EntityPair> entities; 
	
	void addEntity(Entity::Type type, Entity* entity); 
	void updateEntities(); 
	void deleteEntities(); 
}