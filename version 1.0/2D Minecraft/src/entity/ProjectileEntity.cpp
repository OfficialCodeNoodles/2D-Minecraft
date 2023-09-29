#include "../../hdr/entity/ProjectileEntity.hpp"
#include "../../hdr/entity/Player.hpp"
#include "../../hdr/audio/AudioEnviroment.hpp"

namespace engine {
	ProjectileEntity::ProjectileEntity() :
		Entity::Entity(),
		angle(0.0f),
		speed(0.65f), 
		stuckInBlock(false), 
		timeLeft(60 * 20)
	{
		size = gs::Vec2f({ 8.0f / 16.0f, 3.0f / 16.0f }); 
		mass = 0.25f; 
	}
	ProjectileEntity::~ProjectileEntity() {
	}

	void ProjectileEntity::update() {
		const float projectilePlayerDistance = 
			gs::util::distance(position, player->position);

		if ((collisions.floor || collisions.leftWall || collisions.rightWall 
			|| collisions.ceil) && !stuckInBlock) 
		{
			stuckInBlock = true;
			stuckBlockPosition = collisions.blockInsideOf; 
			audio::SoundEvent::soundEvents[audio::SoundEvent::ArrowHit]
				.playSoundEvent(audio::SoundEvent::EventType::Generic,
					projectilePlayerDistance);
		}

		if (!stuckInBlock) {
			gs::util::approach(
				&angle, angle > 270.0f ? 360.0f + 90.0f : 90.0f, 
				std::max(velocity.y * 8.0f, 0.0f)
			);
			gs::util::approach(&speed, 0.0f, 0.5f); 

			const gs::Vec2f speedVector = gs::Vec2f(
				speed * std::cos(gs::util::toRadians(angle)),
				speed * std::sin(gs::util::toRadians(angle))
			); 

			velocity += speedVector; 
			velocity.y += gravitationalStrength * mass; 
			collision::applyCollision(*this, *world, velocity.x < 0.0f);
			updatePosition();

			if (velocity.x != 0.0f)
				velocity.x -= speedVector.x; 
			if (velocity.y != 0.0f)
				velocity.y -= speedVector.y; 
		}
		else {
			if (!world->getBlock(stuckBlockPosition).isSolid()) {
				stuckInBlock = false; 
				velocity = gs::Vec2f(); 
				collisions.clear(); 
				speed = 0.0f;
			}
		}

		dead = timeLeft <= 0;
		timeLeft--;
	}
}