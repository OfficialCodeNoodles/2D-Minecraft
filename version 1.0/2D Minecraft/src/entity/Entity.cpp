#include "../../hdr/entity/Entity.hpp"
#include "../../hdr/entity/Player.hpp"
#include "../../hdr/entity/Mob.hpp"
#include "../../hdr/entity/ItemEntity.hpp"
#include "../../hdr/entity/ProjectileEntity.hpp"
#include "../../hdr/graphics/UI.hpp"

namespace engine {
	Entity::Entity() :
		velocityScaler(1.0f, 1.0f), 
		size({ 1.0f, 1.0f }),
		mass(1.0f),
		immunityFrames(0), 
		dead(false)
	{
		numOfEntities++; 
	}
	Entity::~Entity() {
		numOfEntities--; 
	}

	void Entity::update() {
	}

	std::string Entity::getSaveString() const {
		return "New Entity"
			"\ntype = "
			"\nxpos = " + toString(position.x) +
			"\nypos = " + toString(position.y) +
			"\nwidth = " + toString(size.x) +
			"\nheight = " + toString(size.y) +
			"\nmass = " + toString(mass);
	}

	int Entity::numOfEntities = 0;

	void Entity::updatePosition() {
		position.x += velocity.x * velocityScaler.x;
		position.y += velocity.y * velocityScaler.y; 
		collisions.distanceWalked += std::abs(velocity.x); 
	}

	std::vector<EntityPair> entities;

	void addEntity(Entity::Type type, Entity* entity) {
		switch (type) {
		case Entity::Type::Mob:
			dynamic_cast<Mob*>(entity)->init();
			break; 
		}

		entities.push_back(EntityPair(type, entity)); 
	}
	void updateEntities() {
		audio::SoundEvent::Id playerInteractionSoundEvent = 
			audio::SoundEvent::None;

		for (int entityPairIndex = 0; entityPairIndex < entities.size();
			entityPairIndex++) 
		{
			const EntityPair& entityPair = entities[entityPairIndex];

			Entity::Type entityType = entityPair.first; 
			Entity* entity = entityPair.second; 

			entity->update(); 

			const gs::Vec2f playerEntityDelta = player->position 
				- entity->position;
			const float distance = std::sqrt(
				std::pow(playerEntityDelta.x, 2) 
					+ std::pow(playerEntityDelta.y, 2)
			);

			switch (entityType) {
			case Entity::Type::ItemEntity:
			{
				ItemEntity* itemEntity = dynamic_cast<ItemEntity*>(entity); 

				if (itemEntity->timeLeft < ItemEntity::defaultNumberOfTicksAlive 
					&& collision::intersect(*player, *itemEntity, 0.5f)
					&& !render::ui::gameOver) 
				{
					itemEntity->itemContainer = addItemToInventory(
						itemEntity->itemContainer, player->inventory, 
						Player::numOfInventorySlots
					); 
					audio::SoundEvent::soundEvents[audio::SoundEvent::Pop].
						playSoundEvent(audio::SoundEvent::EventType::ItemPickup); 
				}
			}
				break; 
			case Entity::Type::Mob:
			{
				Mob* mobEntity = dynamic_cast<Mob*>(entity); 
				gs::Vec2f mouseLocation = mouseTileLocation; 

				if (mouseLocation.x < 0.0f)
					mouseLocation.x += 1.0f; 

				if (collision::intersect(*mobEntity, mouseLocation, 0.1f) 
					&& !render::ui::gameOver) 
				{
					player->interactionLocked = true;

					if (gs::input::mouseClickL) {
						if (distance < Player::maxReachDistance) {
							const ItemContainer& itemContainerSelected = 
								player->getItemContainerSelected();
							const int damage = itemContainerSelected.item.isEmpty() ? 1 
								: itemContainerSelected.item.getVar(ItemInfo::damage); 
							const int prvsMobHealth = mobEntity->health; 

							mobEntity->hurt(damage);
							mobEntity->velocity = -collision::calculateKnockback(
								playerEntityDelta
							); 

							if (mobEntity->health < prvsMobHealth) {
								Particle damageParticle(Particle::Type::DamagePoint);

								damageParticle.position = mouseLocation
									- gs::Vec2f(0.0f, 0.5f);
								damageParticle.string = std::to_string(damage);

								addParticle(damageParticle);
							}
						}
					}
				}

				if (collision::intersect(*mobEntity, *player)
					&& !render::ui::gameOver) 
				{
					if (mobEntity->behavior == Mob::Behavior::Hostile) {
						const float mobMeleeDamage = 
							Mob::mobMeleeDamage[mobEntity->type]; 

						player->hurt(mobMeleeDamage); 
						player->velocity = collision::calculateKnockback(
							playerEntityDelta
						); 
					}
				}

				if (mobEntity->dead) {
					const LootTable::Id lootTableId = 
						Mob::mobLootIds[mobEntity->type]; 

					if (lootTableId != LootTable::None) {
						ItemContainer generatedItemContainer; 

						do {
							generatedItemContainer = 
								LootTable::lootTables[lootTableId].getLoot();

							ItemEntity::dropItemEntity(
								generatedItemContainer, mobEntity->position
							);
						} 
						while (!generatedItemContainer.item.isEmpty()); 
					}
				}
			}
				break; 
			case Entity::Type::ProjectileEntity:
			{
				ProjectileEntity* projectileEntity = 
					dynamic_cast<ProjectileEntity*>(entity); 

				if (collision::intersect(*player, *projectileEntity)
					&& !projectileEntity->stuckInBlock) {
					projectileEntity->dead = true; 
					player->hurt(4); 
					player->velocity = collision::calculateKnockback(
						playerEntityDelta
					);
					audio::SoundEvent::soundEvents[audio::SoundEvent::ArrowHit]
						.playSoundEvent(audio::SoundEvent::EventType::Generic);
				}
			}
				break; 
			}

			if (entity->dead) {
				delete entity; 
				entities.erase(entities.begin() + entityPairIndex); 
				entityPairIndex--; 
			}
		}

		//if (playerInteractionSoundEvent == audio::SoundEvent::None 
		//		&& !player->interactionLocked && gs::input::mouseClickL 
		//		&& tileBreakPercentage == 0.0f)
		//	playerInteractionSoundEvent = audio::SoundEvent::WeakHit;

		if (playerInteractionSoundEvent != audio::SoundEvent::None)
			audio::SoundEvent::soundEvents[playerInteractionSoundEvent]
				.playSoundEvent(audio::SoundEvent::EventType::MobHurt); 

		Mob::burnUndeadHostileMobs = world->gameTime.gameTicks > 7000
			&& world->gameTime.gameTicks < 20000; 
	}
	void deleteEntities() {
		for (auto& entityPair : entities) {
			Entity::Type entityType = entityPair.first;
			Entity* entity = entityPair.second;

			delete entity; 
		}

		entities.clear(); 
	}
}