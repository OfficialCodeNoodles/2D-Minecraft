#pragma once

// Dependencies
#include "../world/World.hpp"
#include "../entity/Particle.hpp"

namespace engine {
	class Entity; 

	namespace collision {
		enum class CollisionType { Empty, Block, Fluid };

		struct Collisions {
			bool floor;
			bool ceil;
			bool leftWall;
			bool rightWall;
			bool insideFluid; 
			bool insideWater; 
			bool insideLava; 
			float distanceFallen;
			float distanceWalked;
			gs::Vec2i blockInsideOf; 
			gs::Vec2i floorTile;

			Collisions();
			~Collisions() = default;

			void clear();
		}; 

		bool floor(const Entity& entity, gs::Vec2f position, gs::Vec2f size); 
		bool ceil(const Entity& entity, gs::Vec2f position, gs::Vec2f size); 
		bool leftWall(const Entity& entity, gs::Vec2f position, gs::Vec2f size);
		bool rightWall(const Entity& entity, gs::Vec2f position, gs::Vec2f size); 
		bool intersect(
			const Entity& entity1, const Entity& entity2, 
			float tolerance = 0.0f
		); 
		bool intersect(
			const Entity& entity, gs::Vec2f position, 
			float tolerance = 0.0f
		);

		float calculateEntityDistance(
			const Entity& entity1, const Entity& entity2, bool fromCenter = true
		);
		gs::Vec2f calculateKnockback(gs::Vec2f delta, float multiplier = 1.0f); 
		bool doEntitiesHaveEyeSight(const Entity& entity1, const Entity& entity2); 

		void applyCollision(Entity& entity, const World& world, bool applyForward = true); 
		void applyCollision(Particle& particle, const World& world, bool applyForward = true); 
		void attract(Entity& planetEntity, Entity& moonEntity, bool planetFixed = true); 
	}
}

#include "Entity.hpp"