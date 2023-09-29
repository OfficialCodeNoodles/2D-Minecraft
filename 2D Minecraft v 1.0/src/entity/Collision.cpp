#include "../../hdr/entity/Collision.hpp"
#include "../../hdr/entity/Gravity.hpp"

namespace engine {
	namespace collision {
		Collisions::Collisions() :
			distanceFallen(0.0f),
			distanceWalked(0.0f)
		{
			clear();
		}

		void Collisions::clear() {
			floor = false;
			ceil = false;
			leftWall = false;
			rightWall = false;
			insideFluid = false;
			insideWater = false; 
			insideLava = false; 
			blockInsideOf = gs::Vec2i(-1, -1); 
		}

		bool floor(const Entity& entity, gs::Vec2f position, gs::Vec2f size) {
			const gs::Vec2f halfEntitySize = entity.size * 0.5f; 

			return
				entity.position.x + halfEntitySize.x > position.x
				&& entity.position.x - halfEntitySize.x < position.x + size.x
				&& entity.position.y + halfEntitySize.y >= position.y - entity.velocity.y
				&& entity.position.y + halfEntitySize.y <= position.y + size.y; 
		}
		bool ceil(const Entity& entity, gs::Vec2f position, gs::Vec2f size) {
			const gs::Vec2f halfEntitySize = entity.size * 0.5f; 

			return
				entity.position.x + halfEntitySize.x > position.x
				&& entity.position.x - halfEntitySize.x < position.x + size.x
				&& entity.position.y - halfEntitySize.y <= position.y - entity.velocity.y
				&& entity.position.y - halfEntitySize.y >= position.y - size.y; 
		}
		bool leftWall(const Entity& entity, gs::Vec2f position, gs::Vec2f size) {
			const gs::Vec2f halfEntitySize = entity.size * 0.5f; 

			return
				entity.position.x - halfEntitySize.x <= position.x - entity.velocity.x
				&& entity.position.x - halfEntitySize.x >= position.x - size.x
				&& entity.position.y + halfEntitySize.y > position.y
				&& entity.position.y - halfEntitySize.y < position.y + size.y; 
		}
		bool rightWall(const Entity& entity, gs::Vec2f position, gs::Vec2f size) {
			const gs::Vec2f halfEntitySize = entity.size * 0.5f; 

			return
				entity.position.x + halfEntitySize.x >= position.x - entity.velocity.x
				&& entity.position.x + halfEntitySize.x <= position.x + size.x
				&& entity.position.y + halfEntitySize.y > position.y
				&& entity.position.y - halfEntitySize.y < position.y + size.y; 
		}
		bool intersect(const Entity& entity1, const Entity& entity2, 
			float tolerance) 
		{
			return std::abs(entity1.position.x - entity2.position.x)
					< (entity1.size.x / 2.0f) + (entity2.size.x / 2.0f) + tolerance
				&& std::abs(entity1.position.y - entity2.position.y)
					< (entity1.size.y / 2.0f) + (entity2.size.y / 2.0f) + tolerance;
		}
		bool intersect(const Entity& entity, gs::Vec2f position, float tolerance) {
			return std::abs(entity.position.x - position.x) 
					< (entity.size.x / 2.0f) + tolerance
				&& std::abs(entity.position.y - position.y)
					< (entity.size.y / 2.0f) + tolerance;
		}

		float calculateEntityDistance(
			const Entity& entity1, const Entity& entity2, bool fromCenter) 
		{
			const gs::Vec2f entityDelta = entity1.position - entity2.position;
			return std::sqrt(std::pow(entityDelta.x, 2) 
				+ std::pow(entityDelta.y, 2)); 
		}
		gs::Vec2f calculateKnockback(gs::Vec2f delta, float multiplier) {
			const float distance = gs::util::distance(gs::Vec2f(), delta);
			const gs::Vec2f modifidedDelta = gs::Vec2f(
				delta.x / 3.0f, delta.y / 10.0f
			); 
			return modifidedDelta * (1.0f / std::max(0.05f, distance)) 
				* multiplier;
		}
		bool doEntitiesHaveEyeSight(const Entity& entity1, const Entity& entity2) {
			const float offsetTolerance = 0.05f;

			float deltaX = entity1.position.x - entity2.position.x; 

			if (std::abs(deltaX) < offsetTolerance)
				return true; 

			const float entityEyeSightSlope = 
				(entity1.position.y - entity2.position.y) / deltaX; 

			float tracingSpeed = 0.5f * -gs::util::sign(deltaX); 

			for (gs::Vec2f tracingPosition = entity1.position; 
				std::abs(deltaX) > offsetTolerance;
				tracingPosition += gs::Vec2f(tracingSpeed, tracingSpeed
					* entityEyeSightSlope))
			{
				deltaX = entity1.position.x - entity2.position.x; 

				if (std::abs(deltaX) < std::abs(tracingSpeed))
					tracingSpeed = deltaX; 

				if (world->getBlock(gs::Vec2i(tracingPosition)).isSolid()) 
					return false; 
			}

			return true; 
		}

		void applyCollision(Entity& entity, const World& world, bool applyForward) {
			auto constrainYpos = [](int ypos) -> int {
				return gs::util::clamp(ypos, 0, Chunk::height);
			}; 

			const gs::Vec2i collisionRange = gs::Vec2i(
				std::ceil(entity.size.x / 2.0f) + 2, 
				std::ceil(entity.size.y / 2.0f) + 2
			);
			const float pixelWidth = 1.0f / 16.0f; 
			const gs::Vec2i entityPosition = gs::Vec2i(entity.position); 
			const gs::Vec2f halfEntitySize = entity.size * 0.5f; 

			entity.collisions.clear(); 
			entity.velocityScaler = gs::Vec2f(1.0f, 1.0f); 

			gs::Vec2i tilePosition; 

			for (tilePosition.x = applyForward ? entityPosition.x 
				- collisionRange.x : entityPosition.x + collisionRange.x; 
				applyForward ? tilePosition.x < entityPosition.x 
				+ collisionRange.x : tilePosition.x > entityPosition.x 
				- collisionRange.x; tilePosition.x += applyForward ? 1 : -1)
			{
				for (tilePosition.y = constrainYpos(entityPosition.y
					- collisionRange.y); tilePosition.y < constrainYpos(
						entityPosition.y + collisionRange.y); tilePosition.y++)
				{
					const Block block = world.getBlock(tilePosition); 
					const gs::Vec2f position = gs::Vec2f(tilePosition); 

					if (block.tags.ignoreCollision)
						continue; 

					switch (static_cast<CollisionType>(block.getVar(
						BlockInfo::collisionType)))
					{
					case CollisionType::Block:
					{
						const float floorOffset =
							static_cast<BlockInfo::BlockUpdate>(block.getVar(
								BlockInfo::blockUpdate))
							== BlockInfo::BlockUpdate::FarmLand ? 1.0f
								/ 16.0f : 0.0f;

						if (entity.velocity.y > 0.0f &&
							floor(entity, position + gs::Vec2f(0.0f, floorOffset),
								gs::Vec2f(1.0f, 0.5f)))
						{
							entity.position.y = tilePosition.y 
								- halfEntitySize.y + floorOffset;
							entity.velocity.y = 0.0f;
							entity.collisions.floor = true;
							entity.collisions.blockInsideOf = tilePosition; 
							entity.collisions.floorTile = tilePosition;
						}
						else if (entity.velocity.y < 0.0f && ceil(entity,
							position + gs::Vec2f(0.0f, 1.0f),
							gs::Vec2f(1.0f, 0.5f)))
						{
							entity.position.y = tilePosition.y + 1.0f
								+ halfEntitySize.y;
							entity.velocity.y = 0.0f;
							entity.collisions.ceil = true;
							entity.collisions.blockInsideOf = tilePosition;
						}
						if (entity.velocity.x < 0.0f && leftWall(entity,
							position + gs::Vec2f(1.0f, pixelWidth + floorOffset),
							gs::Vec2f(0.5f, 1.0f - pixelWidth - floorOffset)))
						{
							entity.position.x = tilePosition.x + 1.0f
								+ halfEntitySize.x;
							entity.velocity.x = 0.0f;
							entity.collisions.leftWall = true;
							entity.collisions.blockInsideOf = tilePosition;
						}
						else if (entity.velocity.x > 0.0f && rightWall(entity,
							position + gs::Vec2f(0.0f, pixelWidth + floorOffset), 
							gs::Vec2f(0.5f, 1.0f - pixelWidth - floorOffset)))
						{
							entity.position.x = tilePosition.x
								- halfEntitySize.x;
							entity.velocity.x = 0.0f;
							entity.collisions.rightWall = true;
							entity.collisions.blockInsideOf = tilePosition;
						}
					}
						break;
					case CollisionType::Fluid:
					{
						const float maxFluidLevel = 7.0f;
						const float fluidLevel = block.tags.fluidLevel;
						const gs::Vec2f collisionSize = gs::Vec2f(
							1.0f, 1.0f - (fluidLevel / (maxFluidLevel + 1.0f))
						);
						const float maxFluidFallVelocity = 0.08f;

						if (floor(entity, position + gs::Vec2f(0.0f, 1.0f
							- collisionSize.y), collisionSize))
						{
							entity.velocity.y = std::min(
								entity.velocity.y, maxFluidFallVelocity
							);
							entity.velocityScaler = gs::Vec2f(0.65f, 0.5f);
							entity.collisions.insideFluid = true;
							// Stops entity from taking fall damage in fluids. 
							entity.collisions.distanceFallen = 0.0f;

							if (block.id == Block::Water)
								entity.collisions.insideWater = true; 
							else if (block.id == Block::Lava)
								entity.collisions.insideLava = true;
						}
					}
						break;
					}
				}
			}
		}
		void applyCollision(Particle& particle, const World& world, bool applyForward) {
			Entity hitboxEntity; 

			hitboxEntity.position = particle.position; 
			hitboxEntity.velocity = particle.velocity; 
			hitboxEntity.size = particle.size; 

			applyCollision(hitboxEntity, world, applyForward); 

			particle.position = hitboxEntity.position; 
			particle.velocity = hitboxEntity.velocity; 
		}
		void attract(Entity& planetEntity, Entity& moonEntity, bool planetFixed) {
			const gs::Vec2f bodyDelta = planetEntity.position - moonEntity.position; 
			const float bodyDistance = std::max(
				calculateEntityDistance(planetEntity, moonEntity), 0.5f
			);
			// Calculates how strong the gravitaional attraction should be, 
			// using the formula G * ((m1 * m2) / r^2). 
			const float attractionStrength = -gravitationalStrength
				* ((planetEntity.mass * moonEntity.mass) 
					/ (bodyDistance * bodyDistance)); 
			const gs::Vec2f attractionVelocity = bodyDelta * attractionStrength; 

			moonEntity.velocity -= attractionVelocity; 

			// Doesn't apply gravity to Entity if it is fixed. 
			if (!planetFixed)
				planetEntity.velocity += attractionVelocity; 
		}
	}
}