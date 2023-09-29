#include "../../hdr/entity/Mob.hpp"
#include "../../hdr/util/Random.hpp"
#include "../../hdr/entity/Player.hpp"
#include "../../hdr/entity/ProjectileEntity.hpp"
#include "../../hdr/world/Generation.hpp"

namespace engine {
	Mob::Mob() :
		Entity::Entity(),
		type(Zombie),
		health(20),
		prvsDistanceWalked(0.0f), 
		mobPlayerDistance(0.0f), 
		targetXpos(0.0f),
		jumpAttempts(0),
		targetingPlayer(false),
		burningTicks(0), 
		timeSinceLastAmbientSoundEvent(0),
		skeletonShootingCooldown(0), 
		creeperDetonationTicks(0)
	{
	}
	Mob::Mob(Mob::Type mobType) : Mob() {
		type = mobType; 
		init(); 
	}
	Mob::~Mob() {
		if (behavior == Behavior::Hostile) numOfHostileMobs--;
		else numOfPassiveMobs--;
	}

	void Mob::init() {
		modelTransform.id = mobModelIds[type]; 
		size = mobSizes[type]; 
		behavior = mobBehaviors[type]; 

		if (behavior == Behavior::Hostile) numOfHostileMobs++;
		else numOfPassiveMobs++; 
	}
	void Mob::update() {
		const float targetDeltaThreshold = 0.1f; 
		const float jumpVerticalVelocity = -0.22f; 
		const gs::Vec2f maxPlayerTargetingDelta = gs::Vec2f(30.0f, 10.0f); 
		const int averageAmbientSoundEventFrequency = 60 * 3; 
		const int ambientSoundEventCooldown = 60; 

		const audio::SoundEvent::Id ambientSoundEvent = mobAmbientSoundEvents[type]; 
		const gs::Vec2f mobPlayerDelta = gs::Vec2f(
			std::abs(position.x - player->position.x), 
			std::abs(position.y - player->position.y)
		);
		
		mobPlayerDistance = gs::util::distance(gs::Vec2f(), mobPlayerDelta); 

		// Handle ambient sound events. 
		if (ambientSoundEvent != audio::SoundEvent::None
			&& timeSinceLastAmbientSoundEvent == 0) 
		{
			if (randomGenerator.generate() % averageAmbientSoundEventFrequency == 0) {
				audio::SoundEvent::soundEvents[ambientSoundEvent]
					.playSoundEvent(audio::SoundEvent::EventType::MobAmbience, 
						mobPlayerDistance);
				timeSinceLastAmbientSoundEvent = ambientSoundEventCooldown; 
			}
		}

		float horizontalSpeed = 0.1f; 

		applyGravity(*this);

		switch (type) {
		case Zombie:
		{
			const int playerTargetingChange = gs::util::clamp(
				static_cast<int>(mobPlayerDistance * 20), 120, 1200
			);

			horizontalSpeed = 0.03f;

			if (targetingPlayer) {
				targetXpos = player->position.x;
				horizontalSpeed = 0.04f;

				if (mobPlayerDelta.x > maxPlayerTargetingDelta.x
					|| mobPlayerDelta.y > maxPlayerTargetingDelta.y
					|| randomGenerator.generate() % 600 == 0)
				{
					targetingPlayer = false;
				}
			}
			else if (mobPlayerDelta.x < maxPlayerTargetingDelta.x
				&& mobPlayerDelta.y < maxPlayerTargetingDelta.y
				&& randomGenerator.generate() % playerTargetingChange == 0)
				//&& collision::doEntitiesHaveEyeSight(*this, *player))
			{
				targetingPlayer = true;
			}
			else if (randomGenerator.generate() % 300 == 0)
				targetXpos = position.x + (randomGenerator.generate() % 50) 
					- 25;
		}
			break; 
		case Skeleton:
			horizontalSpeed = 0.05f;

			if (randomGenerator.generate() % 300 == 0)
				targetXpos = position.x + (randomGenerator.generate() % 50) 
					- 25;

			if (skeletonShootingCooldown == 0 && mobPlayerDistance < 15.0f) {
				skeletonShootingCooldown = 180; 

				ProjectileEntity* arrow = new ProjectileEntity(); 

				arrow->position = position + gs::Vec2f(
					1.0f * (modelTransform.facingForward ? 1.0f : -1.0f), 
					-0.5f
				); 
				arrow->angle = gs::util::angleBetween(
					position, player->position
				);
				arrow->angle += arrow->angle > 270 ? -8.0f : 8.0f; 

				addEntity(Entity::Type::ProjectileEntity, arrow); 
				audio::SoundEvent::soundEvents[audio::SoundEvent::ArrowShoot]
					.playSoundEvent(audio::SoundEvent::EventType::Generic,
						mobPlayerDistance);
			} 

			skeletonShootingCooldown = std::max(
				skeletonShootingCooldown - 1, 0
			); 

			break; 
		case Creeper:
		{
			const float creeperExplosionRadius = 5.0f;

			horizontalSpeed = 0.03f;

			if (randomGenerator.generate() % 300 == 0)
				targetXpos = position.x + (randomGenerator.generate() % 50)
					- 25;

			if (mobPlayerDistance < creeperExplosionRadius) {
				const int maxCreeperDetonationTicks = 120; 

				if (creeperDetonationTicks == 0)
					creeperDetonationTicks = maxCreeperDetonationTicks;
				else if (creeperDetonationTicks == 1) {
					player->hurt(std::max(std::pow(creeperExplosionRadius, 2.0f) 
						- ((mobPlayerDistance - 1.0f) * creeperExplosionRadius), 0.0f));
					dead = true;

					for (auto& blockPosition : 
							generateCircleBlocks(gs::Vec2i(position), 5.0f)) 
						world->breakBlock(blockPosition, false); 

					audio::SoundEvent::soundEvents[audio::SoundEvent::Explode].
						playSoundEvent(audio::SoundEvent::EventType::Generic, 
							mobPlayerDistance);
				}

				creeperDetonationTicks--;
				velocity.x = 0.0f;  
				horizontalSpeed = 0.0f; 
				modelTransform.setColor(gs::util::approach(
					gs::Color::Red, gs::Color::White, 
					100.0f * (static_cast<float>(creeperDetonationTicks) 
						/ static_cast<float>(maxCreeperDetonationTicks))
				));
			}
			else
				creeperDetonationTicks = 0; 

		}
			break; 
		case Type::Pig:
			horizontalSpeed = 0.02f; 
			if (randomGenerator.generate() % 300 == 0)
				targetXpos = position.x 
					+ (randomGenerator.generate(false) % 20); 
			break; 
		case Type::Cow:
			horizontalSpeed = 0.02f;
			if (randomGenerator.generate() % 300 == 0)
				targetXpos = position.x
					+ (randomGenerator.generate(false) % 20);
			break; 
		}

		if (behavior == Behavior::Hostile && mobUndead[type]
			&& burnUndeadHostileMobs && burningTicks % 60 == 0)
		{
			if (world->isBlockExposedToSky(gs::Vec2i(position)) 
					&& !collisions.insideWater) 
				burningTicks = 180; 
		}

		const int maxJumpAttempts = 3; 
		const float targetDeltaXpos = targetXpos - position.x; 

		if (targetDeltaXpos > targetDeltaThreshold) {
			gs::util::approach(&velocity.x, std::min(
				targetDeltaXpos, horizontalSpeed), 20.0f);
			modelTransform.facingForward = true; 

			if (collisions.floor && collisions.rightWall) {
				velocity.y = jumpVerticalVelocity;
				jumpAttempts++; 

				if (jumpAttempts > maxJumpAttempts) {
					targetXpos = position.x; 
					jumpAttempts = 0; 
				}
			}
		}
		else if (targetDeltaXpos < -targetDeltaThreshold) {
			gs::util::approach(&velocity.x, std::max(
				targetDeltaXpos, -horizontalSpeed), 20.0f);
			modelTransform.facingForward = false;	 

			if (collisions.floor && collisions.leftWall) {
				velocity.y = jumpVerticalVelocity;
				jumpAttempts++; 

				if (jumpAttempts > maxJumpAttempts) {
					targetXpos = position.x;
					jumpAttempts = 0;
				}
			}
		}	
		else
			gs::util::approach(&velocity.x, 0.0f, 20.0f);

		const float distanceRequiredForWalkingSound = 1.0f; 

		if (collisions.distanceWalked - prvsDistanceWalked >= 
			distanceRequiredForWalkingSound) 
		{
			const audio::SoundEvent::Id walkingSoundEvent = 
				mobWalkingSoundEvents[type];

			audio::SoundEvent::soundEvents[walkingSoundEvent].playSoundEvent(
				audio::SoundEvent::EventType::Generic, mobPlayerDistance
			);
			prvsDistanceWalked = collisions.distanceWalked; 
		}

		if (collisions.floor) {
			hurt(std::max(static_cast<int>(collisions.distanceFallen 
				- 4.0f), 0)); 
			collisions.distanceFallen = 0.0f; 
		}
		else
			collisions.distanceFallen += velocity.y;

		if (collisions.insideLava) {
			if (burningTicks % 60 == 0)
				burningTicks = 180;
		}

		if (burningTicks > 0) {
			if (burningTicks % 60 == 0)
				hurt(collisions.insideLava ? 7 : 1);

			modelTransform.setColor(gs::Color(255, 127, 0));
		}

		if (prvsPosition.x != position.x)
			jumpAttempts = 0; 

		if (health == 0)
			dead = true; 

		collision::applyCollision(*this, *world, velocity.x < 0.0f);
		updatePosition();

		immunityFrames = std::max(immunityFrames - 1, 0);
		prvsPosition = position; 
		modelTransform.update(*this);
		burningTicks = std::max(burningTicks - 1, 0);
		timeSinceLastAmbientSoundEvent = std::max(timeSinceLastAmbientSoundEvent - 1, 0);
	}
	void Mob::hurt(int damage) {
		if (damage > 0 && immunityFrames == 0) {
			const audio::SoundEvent::Id hurtSoundEvent =
				mobHurtSoundEvents[type];
			const audio::SoundEvent::Id deathSoundEvent =
				mobDeathSoundEvents[type]; 

			health = std::max(health - damage, 0);
			immunityFrames = 20; 
			modelTransform.hurt();

			const audio::SoundEvent::Id selectedSoundEvent = health == 0 
				? deathSoundEvent : hurtSoundEvent; 

			if (selectedSoundEvent != audio::SoundEvent::None) {
				audio::SoundEvent::soundEvents[selectedSoundEvent].playSoundEvent(
					audio::SoundEvent::EventType::MobHurt, mobPlayerDistance
				); 
			}
		}
	}

	std::string Mob::getSaveString() const {
		std::string saveString = Entity::getSaveString();

		const size_t symbolOffset = saveString.find_first_of('=');
		saveString.insert(
			symbolOffset + 1, " " + toString(
				static_cast<int>(Entity::Type::Mob))
		);

		return saveString +
			"\nmobType = " + toString(type);
	}

	const Model::Id Mob::mobModelIds[End] = { 
		Model::Id::Zombie,			Model::Id::Skeleton,
		Model::Id::Creeper,			Model::Id::Pig, 
		Model::Id::Cow
	};
	const gs::Vec2f Mob::mobSizes[End] = { 
		gs::Vec2f(4.0f / 16.0f, 30.0f / 16.0f),
		gs::Vec2f(4.0f / 16.0f, 30.0f / 16.0f),
		gs::Vec2f(8.0f / 16.0f, 24.0f / 16.0f),
		gs::Vec2f(26.0f / 16.0f, 22.0f / 16.0f),
		gs::Vec2f(24.0f / 16.0f, 24.0f / 16.0f)
	};
	const Mob::Behavior Mob::mobBehaviors[End] = { 
		Mob::Behavior::Hostile,		Mob::Behavior::Hostile,
		Mob::Behavior::Hostile,		Mob::Behavior::Passive,
		Mob::Behavior::Passive
	};
	const bool Mob::mobUndead[End] = {
		true,						true,
		false,						false,
		false
	};
	const LootTable::Id Mob::mobLootIds[End] = { 
		LootTable::ZombieDrops,		LootTable::SkeletonDrops,
		LootTable::CreeperDrops,	LootTable::PigDrops,
		LootTable::CowDrops
	}; 
	const float Mob::mobMeleeDamage[End] = {
		4.0f,						2.0f, 
		0.0f,						0.0f,
		0.0f, 
	}; 
	const audio::SoundEvent::Id Mob::mobWalkingSoundEvents[End] = {
		audio::SoundEvent::ZombieWalk, audio::SoundEvent::SkeletonWalk,
		audio::SoundEvent::ZombieWalk, audio::SoundEvent::None,
		audio::SoundEvent::CowWalk
	};
	const audio::SoundEvent::Id Mob::mobAmbientSoundEvents[End] = {
		audio::SoundEvent::ZombieAmbient, audio::SoundEvent::SkeletonAmbient,
		audio::SoundEvent::None, audio::SoundEvent::PigAmbient,
		audio::SoundEvent::CowAmbient
	};
	const audio::SoundEvent::Id Mob::mobHurtSoundEvents[End] = {
		audio::SoundEvent::ZombieHurt, audio::SoundEvent::SkeletonHurt,
		audio::SoundEvent::CreeperHurt, audio::SoundEvent::PigAmbient,
		audio::SoundEvent::CowHurt
	}; 
	const audio::SoundEvent::Id Mob::mobDeathSoundEvents[End] = {
		audio::SoundEvent::ZombieDeath, audio::SoundEvent::SkeletonDeath,
		audio::SoundEvent::CreeperDeath, audio::SoundEvent::PigDeath,
		audio::SoundEvent::CowAmbient
	}; 
	const int Mob::hostileMobSpawnCap = 30, 
		Mob::passiveMobSpawnCap = 8;
	const int Mob::hostileMobUnloadDistance = World::chunkLoadDistance * Chunk::width; 
	int Mob::numOfHostileMobs = 0, 
		Mob::numOfPassiveMobs = 0;
	bool Mob::burnUndeadHostileMobs = false; 
}