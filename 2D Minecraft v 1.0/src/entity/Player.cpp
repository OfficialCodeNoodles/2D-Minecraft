#include "../../hdr/entity/Player.hpp"
#include "../../hdr/entity/ItemEntity.hpp"
#include "../../hdr/entity/Collision.hpp"
#include "../../hdr/graphics/UI.hpp"
#include "../../hdr/audio/AudioEnviroment.hpp"
#include "../../hdr/util/Random.hpp"

namespace engine {
	Player::Player() :
		Entity::Entity(),
		health(maxHealth), 
		foodLevel(maxFoodLevel), 
		crouching(false),
		miningDuration(0.0f), 
		interactionLocked(false),
		placeOnGround(true), 
		foodSaturationLevel(5.0f), 
		foodExhaustionLevel(0.0f), 
		foodTickTimer(0), 
		burningTicks(0), 
		crouchingTile(-1, -1)
	{
		modelTransform.id = Model::Player; 
		position.x = 0.5f; 
		size = gs::Vec2f(4.0f / 16.0f, 30.0f / 16.0f);
	}
	Player::~Player() {
		//Entity::~Entity(); 
	}

	void Player::update() {
		if (render::ui::gameOver)
			return; 

		const float horizontalSpeed = 0.15f;
		const float backwardsSpeedScaler = 0.5f; 
		const float crouchingSpeedScaler = 0.5f; 
		const float eyeline = 26.0f; 

		// Calculate angle between player and mouse. 

		float& headAngle = modelTransform.transforms[ModelTransform::Head].angle;

		headAngle = gs::util::angleBetween(
			gs::input::mousePosition, 
			render::transformPosition(position + gs::Vec2f(0.0f, 
				(size.y / 2.0f) - (eyeline / 16.0f)))
		); 
		headAngle -= 180.0f; 

		modelTransform.calculateDirection(); 

		// Find which tile the player is focused on. 

		gs::Vec2f transformedMousePosition = gs::input::mousePosition;
		
		if (-transformedMousePosition.x > render::cameraPosition.x)
			transformedMousePosition.x -= render::scaleValue(1.0f); 

		mouseTileLocation = gs::Vec2f(
			(render::cameraPosition.x + transformedMousePosition.x)
				/ render::scaleValue(1.0f),
			(render::cameraPosition.y + transformedMousePosition.y) 
				/ render::scaleValue(1.0f)
		); 

		if (!interactionLocked)
			mouseTilePosition = gs::Vec2i(mouseTileLocation.x, mouseTileLocation.y);
		else
			resetTileFocus(); 

		if (input::isKeyClicked(input::SwitchTileMode)) {
			blockInteraction = !blockInteraction;
			resetTileFocus();
			audio::playSoundEffect(audio::SoundEffect::SwitcherClick); 
		}

		foodSaturationLevel = std::max(foodSaturationLevel - foodExhaustionLevel, 0.0f); 
		foodExhaustionLevel = 0.005f; 

		if (foodSaturationLevel == 0.0f) {
			foodSaturationLevel = 5.0f;
			foodLevel--; 
			foodTickTimer--;
		}

		if (health < maxHealth && foodLevel > 17) {
			if (foodTickTimer <= 0) {
				health++; 
				foodExhaustionLevel = 1.0f; 
				foodTickTimer = 60 * 2; 
			}

			foodTickTimer--;
		}

		// Spawns the player on the ground when a new world is created. 
		if (placeOnGround) [[unlikely]] {
			if (world->getChunk(World::getChunkOffset(position.x)) != nullptr) {
				while (position.y < Chunk::height) {
					const gs::Vec2i footPosition = gs::Vec2i(
						position.x, position.y + std::ceil(size.y / 2.0f)
					);

					if (static_cast<collision::CollisionType>(world->getBlock(
						footPosition).getVar(BlockInfo::collisionType))
							!= collision::CollisionType::Block)
						position.y++;
					else
						break;
				}

				placeOnGround = false;
			}
		}

		switch (gameMode) {
		case GameMode::Survival:
		{
			const float playerTileDistance = std::abs(position.x
				- mouseTilePosition.x - 0.5f) + std::abs(position.y
					- mouseTilePosition.y - 0.5f);

			// Stop tile from being reached if it's too far away. 
			if (playerTileDistance > maxReachDistance)
				resetTileFocus();

			applyGravity(*this);

			// Handle vertical velocity.
			if ((collisions.floor && input::isKeyPressed(input::Jump)) 
				|| (collisions.insideFluid && input::isKeyClicked(input::Jump)))
			{
				velocity.y = -0.31f;
				crouchingTile = gs::Vec2i(-1, -1);
				foodExhaustionLevel = 0.05f; 
			}

			crouching = input::isKeyPressed(input::Crouch);

			// Handle horizontal velocity. 

			float currentHorizontalSpeed = horizontalSpeed;

			if (crouching) currentHorizontalSpeed *= crouchingSpeedScaler;

			if (input::isKeyPressed(input::MoveLeft)) {
				velocity.x = -currentHorizontalSpeed
					* (!modelTransform.facingForward ? 1 : backwardsSpeedScaler);
				modelTransform.targetVelocity.x = velocity.x;
			}
			if (input::isKeyPressed(input::MoveRight)) {
				velocity.x = currentHorizontalSpeed
					* (modelTransform.facingForward ? 1 : backwardsSpeedScaler);
				modelTransform.targetVelocity.x = velocity.x;
			}

			gs::util::approach(&velocity.x, 0.0f, 20.0f);

			collision::applyCollision(*this, *world, velocity.x < 0.0f);
			updatePosition();

			if (collisions.floor) {
				if (collisions.distanceFallen > 5.0f)
					audio::playSoundEffect(audio::SoundEffect::LargeFall);
				else if (collisions.distanceFallen > 3.0f)
					audio::playSoundEffect(audio::SoundEffect::SmallFall); 

				hurt(std::max(static_cast<int>(collisions.distanceFallen 
					- 4.0f), 0), false);
				collisions.distanceFallen = 0.0f; 
			}
			else
				collisions.distanceFallen += velocity.y;

			if (crouching) {
				// Only apply crouching bounds if standing on valid floor tile.
				if (crouchingTile != gs::Vec2i(-1, -1)) {
					const gs::Vec2f xBounds = gs::Vec2f(
						crouchingTile.x - (size.x / 2.1f),
						crouchingTile.x + (size.x / 2.1f) + 1.0f
					);

					if (!gs::util::inBound(position.x, xBounds.x, xBounds.y))
						gs::util::clamp(&position.x, xBounds.x, xBounds.y);
				}

				// Changes what tile to lock to, when on the ground.
				if (collisions.floor)
					crouchingTile = collisions.floorTile;
			}
			else
				crouchingTile = gs::Vec2i(-1, -1);

			if (collisions.insideLava) {
				if (burningTicks % 60 == 0)
					burningTicks = 180; 
			}

			if (burningTicks > 0) {
				if (burningTicks % 60 == 0)
					hurt(collisions.insideLava ? 7 : 1);

				modelTransform.setColor(gs::Color(255, 127, 0)); 
			}

			// Resets all empty items. 
			for (auto& itemContainer : inventory) {
				if (itemContainer.count == 0)
					itemContainer.item = Item();
			}

			ItemContainer& itemContainerSelected = getItemContainerSelected(); 

			const ItemInfo& itemInfo = ItemInfo::itemInfo[
				itemContainerSelected.item.id];
			const ItemInfo::PlacementType itemSelectedType =
				static_cast<ItemInfo::PlacementType>(
					itemInfo.getVar(ItemInfo::type));

			if (!render::ui::inventoryOpen) {
				// Handle tile breaking. 

				const float armSwingDuration = std::pow(gs::util::PI, 2) / 2.0f;

				// Stops mining process if tile position changes. 
				if (tileBeingBroken != mouseTilePosition) {
					//tileBreakDuration = 0; 
					tileBreakPercentage = 0.0f;
				}

				const Block currentBlock = world->getBlock(mouseTilePosition);
				const Wall currentWall = world->getWall(mouseTilePosition);

				if (gs::input::activeMouseClickL) {
					// Handle block breaking. 
					if (!interactionLocked && blockInteraction
						&& !currentBlock.isEmpty() && !currentBlock.isFluid()) 
					{
						const bool isValidTool = currentBlock.getVar(BlockInfo::requiredToolToBreak)
							== itemInfo.getVar(ItemInfo::toolType)
								|| currentBlock.getVar(BlockInfo::requiredToolToBreak)
								== static_cast<float>(ItemInfo::ToolType::None);
						const audio::SoundEvent::Id soundEventId = static_cast<audio::SoundEvent::Id>(
							currentBlock.getVar(BlockInfo::soundEvent)
						); 
						const float itemMineSpeed = isValidTool ?
							itemInfo.getVar(ItemInfo::toolMineSpeed)
								: ItemInfo::defaultToolMineSpeed;

						tileBeingBroken = mouseTilePosition;
						tileBreakDuration++; 
						tileBreakPercentage += itemMineSpeed / std::max(
							currentBlock.getVar(BlockInfo::hardness), 0.01f
						);
						miningDuration += 1.0f;

						if (soundEventId != audio::SoundEvent::None
							&& (tileBreakDuration - 1) % audio::SoundEvent::soundEventCooldownDuration == 0
							&& tileBreakPercentage < 100.0f)
						{
							audio::SoundEvent::soundEvents[soundEventId].playSoundEvent(
								audio::SoundEvent::EventType::Mining, playerTileDistance
							);
						}

						if (render::shouldParticlesBeRendered)
							Particle::generateBlockParticles(tileBeingBroken);

						// Break block if break percentage reaches 100%. 
						if (tileBreakPercentage >= 100.0f) {
							bool dropItem = itemInfo.getVar(ItemInfo::toolStrength)
								>= currentBlock.getVar(BlockInfo::requiredToolStrength);

							// Checks if the correct tool was used. 
							if (dropItem && !currentBlock.getVar(
									BlockInfo::dropItemWithIncorrectTool))
								dropItem = dropItem || isValidTool;

							world->breakBlock(tileBeingBroken, dropItem, true);
							tileBreakDuration = 0; 
							tileBreakPercentage = 0.0f;
							itemContainerSelected.item.tags.durability--; 

							// Handle tool breaking. 
							if (itemContainerSelected.item.tags.durability <= 0) {
								itemContainerSelected = ItemContainer(Item());
								audio::playSoundEffect(audio::SoundEffect::Break);
							}
						}
					}
					// Handle wall breaking. 
					else if (!interactionLocked && !blockInteraction
						&& !currentWall.isEmpty())
					{
						const bool isValidTool = currentWall.getVar(WallInfo::requiredToolToBreak)
							== itemInfo.getVar(ItemInfo::toolType)
								|| currentWall.getVar(WallInfo::requiredToolToBreak)
								== static_cast<float>(ItemInfo::ToolType::None);
						const audio::SoundEvent::Id soundEventId = static_cast<audio::SoundEvent::Id>(
							currentWall.getVar(WallInfo::soundEvent)
						);
						const float itemMineSpeed = isValidTool ?
							itemInfo.getVar(ItemInfo::toolMineSpeed)
								: ItemInfo::defaultToolMineSpeed;

						tileBeingBroken = mouseTilePosition;
						tileBreakDuration++; 
						tileBreakPercentage += itemMineSpeed / std::max(
							currentWall.getVar(WallInfo::hardness), 0.01f
						);
						miningDuration += 1.0f;

						if (soundEventId != audio::SoundEvent::None
							&& (tileBreakDuration - 1) % audio::SoundEvent::soundEventCooldownDuration == 0
							&& tileBreakDuration < 100.0f)
						{
							audio::SoundEvent::soundEvents[soundEventId].playSoundEvent(
								audio::SoundEvent::EventType::Mining, playerTileDistance
							);
						}

						if (render::shouldParticlesBeRendered)
							Particle::generateWallParticles(tileBeingBroken);

						// Break wall if break percentage reaches 100%. 
						if (tileBreakPercentage >= 100.0f) {
							bool dropItem = itemInfo.getVar(ItemInfo::toolStrength)
								>= currentWall.getVar(WallInfo::requiredToolStrength);

							// Checks if the correct tool was used. 
							if (dropItem && !currentWall.getVar(
									WallInfo::dropItemWithIncorrectTool))
								dropItem = dropItem || isValidTool;

							world->breakWall(tileBeingBroken, dropItem, true);
							tileBreakDuration = 0;
							tileBreakPercentage = 0.0f;
							itemContainerSelected.item.tags.durability--;
							
							// Handle tool breaking. 
							if (itemContainerSelected.item.tags.durability <= 0) {
								itemContainerSelected = ItemContainer(Item());
								audio::playSoundEffect(audio::SoundEffect::Break); 
							}
						}
					}
					else {
						tileBreakDuration = 0;
						tileBreakPercentage = 0.0f;
						miningDuration = gs::input::mouseClickL
							? armSwingDuration : 0.0f;
					}
				}
				else {
					tileBreakDuration = 0; 
					tileBreakPercentage = 0.0f;
					miningDuration = 0.0f;
				}

				if (gs::input::activeMouseClickR) {
					// Handle tile placement. 
					if (tilePlacementCooldown == 0
						&& itemContainerSelected.count > 0)
					{
						// Block placement. 
						if (itemSelectedType == ItemInfo::PlacementType::Block) {
							const Block::Id blockId = static_cast<Block::Id>(
								itemInfo.getVar(ItemInfo::tileIndex));
							const collision::CollisionType blockCollision =
								static_cast<collision::CollisionType>(
									BlockInfo::blockInfo[blockId].getVar(
										BlockInfo::collisionType));

							bool safeToPlace = true;

							// Checks to make sure a block isn't placed inside
							// of the player. 
							if (blockCollision == collision::CollisionType::Block) {
								Entity blockEntity;

								blockEntity.size = gs::Vec2f(1.0f, 1.0f);
								blockEntity.position = gs::Vec2f(
									mouseTilePosition.x + 0.5f,
									mouseTilePosition.y + 0.5f
								);

								if (collision::intersect(*this, blockEntity))
									safeToPlace = false;
							}

							if (world->isValidBlockPlacementLocation(
								mouseTilePosition, blockId) && safeToPlace)
							{
								world->placeBlock(
									mouseTilePosition, blockId, 
									World::PlaceFilter::Replace, true
								);
								itemContainerSelected.count--;

								tilePlacementCooldown =
									tilePlacementCooldownDuration;
								miningDuration = armSwingDuration;
							}
						}
						// Wall placement. 
						else if (itemSelectedType == ItemInfo::PlacementType::Wall) {
							const Wall::Id wallId = static_cast<Wall::Id>(
								itemInfo.getVar(ItemInfo::tileIndex));

							if (world->isValidWallPlacementLocation(
								mouseTilePosition, wallId))
							{
								world->placeWall(
									mouseTilePosition, wallId,
									World::PlaceFilter::Replace, true
								); 
								itemContainerSelected.count--; 

								tilePlacementCooldown =
									tilePlacementCooldownDuration;
								miningDuration = armSwingDuration;
							}
						}
					}

					// Handle food eating.

					const int foodPoints = itemInfo.getVar(ItemInfo::foodPoints); 

					if (foodLevel < maxFoodLevel && foodPoints > 0) {
						// Eats food once the cooldown has been depleted. 
						if (foodEatingCooldown <= 0) {
							const float foodSaturation = 
								itemInfo.getVar(ItemInfo::foodSaturation); 

							foodLevel = std::min(maxFoodLevel, 
								foodLevel + foodPoints);
							foodSaturationLevel += foodSaturation; 

							foodEatingCooldown = foodEatingCooldownDuration; 
							itemContainerSelected.count--; 

							audio::SoundEvent::soundEvents[audio::SoundEvent::Burp].
								playSoundEvent(audio::SoundEvent::EventType::Generic);
						}
						else {
							foodEatingCooldown--; 

							if (foodEatingCooldown % 12 == 0)
								audio::SoundEvent::soundEvents[audio::SoundEvent::Eating].
									playSoundEvent(audio::SoundEvent::EventType::Generic); 
						}
					}
					else
						foodEatingCooldown = foodEatingCooldownDuration; 

					if (gs::input::mouseClickR) {
						const ItemInfo::ToolType toolType =
							static_cast<ItemInfo::ToolType>(
								itemInfo.getVar(ItemInfo::toolType));
						const render::ui::InventoryMenu blockInventoryMenu =
							static_cast<render::ui::InventoryMenu>(
								currentBlock.getVar(BlockInfo::inventoryMenu));
						
						switch (toolType) {
						case ItemInfo::ToolType::Hoe:
						{
							Block& blockSelected = 
								world->getBlockRef(mouseTilePosition); 

							if (blockSelected.id == Block::Dirt
								|| blockSelected.id == Block::GrassBlock)
							{
								world->placeBlock(
									mouseTilePosition, Block(Block::FarmLand), 
									World::PlaceFilter::Replace, true
								); 
								itemContainerSelected.item.tags.durability--;

								// Handle tool breaking. 
								if (itemContainerSelected.item.tags.durability <= 0) {
									itemContainerSelected = ItemContainer(Item());
									audio::playSoundEffect(audio::SoundEffect::Break);
								}
							}
						}
							break; 
						}

						// Handle block menu interaction. 
						if (blockInventoryMenu != render::ui::InventoryMenu::None) {
							render::ui::inventoryOpen = true;
							render::ui::inventoryMenu = blockInventoryMenu;

							tileEntityPosition = mouseTilePosition;
						}

						switch (itemContainerSelected.item.id) {
						case Item::Bucket:
							if (currentBlock.isFluid() 
								&& currentBlock.tags.isFluidSource) 
							{
								const ItemContainer filledBucket(
									currentBlock.id == Block::Water 
										? Item::WaterBucket : Item::LavaBucket, 
									1
								); 

								itemContainerSelected.count--; 
								addItemToInventory(
									filledBucket, inventory, 
									numOfInventorySlots
								); 

								world->breakBlock(mouseTilePosition, false); 
							}
							break; 
						case Item::WaterBucket:
							if (currentBlock.isEmpty() 
								&& world->isValidBlockPlacementLocation(
									mouseTilePosition, Block::Water))
							{
								itemContainerSelected.item.id = Item::Bucket;
								world->placeBlock(mouseTilePosition, Block::Water); 
							}
							break; 
						case Item::LavaBucket:
							if (currentBlock.isEmpty()
								&& world->isValidBlockPlacementLocation(
									mouseTilePosition, Block::Lava))
							{
								itemContainerSelected.item.id = Item::Bucket;
								world->placeBlock(mouseTilePosition, Block::Lava);
							}
							break; 
						}
					}
				}

				// Handle tile dropping. 
				if (input::isKeyClicked(input::DropItem)) {
					if (itemContainerSelected.count > 0) {
						const float horizontalOffset = 0.3f;
						const float horizontalVelocity = 0.4f;

						gs::Vec2f itemPosition = position;
						gs::Vec2f itemVelocity = velocity;

						itemPosition.x += modelTransform.facingForward ? horizontalOffset
							: -horizontalOffset;
						itemVelocity.x += modelTransform.facingForward ? horizontalVelocity
							: -horizontalVelocity;

						ItemEntity::dropItemEntity(
							ItemContainer(itemContainerSelected.item, 1),
							itemPosition, itemVelocity, true
						);

						// Reduce item count. 
						itemContainerSelected.count--;

						audio::SoundEvent::soundEvents[audio::SoundEvent::Pop].
							playSoundEvent(audio::SoundEvent::EventType::Generic);
					}
				}
			}
			else {
				tileBreakDuration = 0;
				tileBreakPercentage = 0.0f;
				miningDuration = 0.0f;
			}

			if (foodLevel == 0) {
				if (render::window::ticks % 120 == 0)
					hurt(1); 
			}

			if (health == 0) {
				render::ui::gameOver = true;

				for (auto& itemContainer : inventory) {
					const gs::Vec2f velocity = gs::Vec2f(
						static_cast<float>(randomGenerator.generate(false) % 1000)
							/ 1000.0f, 
						static_cast<float>(randomGenerator.generate(false) % 1000)
							/ 2000.0f
					);

					ItemEntity::dropItemEntity(
						itemContainer, position, velocity
					); 
					itemContainer = ItemContainer(Item()); 
				}
			}
			
			break; 
		}
		case GameMode::Creative:
			collisions.distanceFallen = 0.0f; 

			break; 
		case GameMode::Spectator:
			collisions.distanceFallen = 0.0f;
			runFreeCamera(); 
			break; 
		}

		immunityFrames = std::max(immunityFrames - 1, 0); 
		tilePlacementCooldown = std::max(tilePlacementCooldown - 1, 0);
		burningTicks = std::max(burningTicks - 1, 0);
		interactionLocked = false; 
		modelTransform.update(*this);
	}
	void Player::hurt(int damage, bool playSound) {
		if (damage > 0 && immunityFrames == 0) {
			health = std::max(health - damage, 0);
			immunityFrames = 60; 
			foodExhaustionLevel = 0.1f; 
			modelTransform.hurt();

			if (playSound)
				audio::SoundEvent::soundEvents[audio::SoundEvent::Hurt].
					playSoundEvent(audio::SoundEvent::EventType::Generic);
		}
	}

	ItemContainer& Player::getItemContainerSelected() {
		return inventory[render::ui::hotbarIndex]; 
	}
	std::string Player::getSaveString() const {
		std::string saveString = Entity::getSaveString(); 

		const size_t symbolOffset = saveString.find_first_of('=');
		saveString.insert(
			symbolOffset + 1, " " + toString(
				static_cast<int>(Entity::Type::PlayerEntity))
		);

		saveString += "\nhealth = " + toString(health);
		saveString += "\nfoodLevel = " + toString(foodLevel); 
		saveString += "\nitems = ";

		// Add items to save string. 
		for (int itemSlot = 0; itemSlot < numOfInventorySlots; itemSlot++) {
			const ItemContainer& itemContainer = inventory[itemSlot];
			saveString += toString(itemContainer.item.id) + ":" 
				+ toString(itemContainer.item.tags.asInt) + ":" 
				+ toString(itemContainer.count) + " ";
		}

		return saveString; 
	}

	const int Player::maxHealth = 20; 
	const int Player::maxFoodLevel = 20;
	const int Player::foodEatingCooldownDuration = 120; 
	const int Player::numOfHotbarSlots = 9;
	const float Player::maxReachDistance = 5.0f;
	const int Player::tilePlacementCooldownDuration = 6; 

	void Player::runFreeCamera() {
		const float cameraPanSpeed = 0.5f;

		// Horizontal movements. 
		if (input::isKeyPressed(input::MoveLeft)) 
			position.x -= cameraPanSpeed;
		else if (input::isKeyPressed(input::MoveRight)) 
			position.x += cameraPanSpeed;

		// Vertical movements. 
		if (input::isKeyPressed(input::MoveUp))
			position.y -= cameraPanSpeed;
		else if (input::isKeyPressed(input::MoveDown))
			position.y += cameraPanSpeed;
	}

	void Player::resetTileFocus() {
		mouseTilePosition = gs::Vec2i(-1, -1);
		tileBreakPercentage = 0.0f; 
	}

	Player* player = nullptr; 
	gs::Vec2f mouseTileLocation;
	gs::Vec2i mouseTilePosition;
	bool blockInteraction = true; 
	gs::Vec2i tileBeingBroken;
	int tileBreakDuration = 0;
	float tileBreakPercentage = 0.0f; 
	int tilePlacementCooldown = 0; 
	gs::Vec2i tileEntityPosition; 
	int foodEatingCooldown = Player::foodEatingCooldownDuration;

	void createPlayer() {
		Entity::numOfEntities = 0; 
		player = new engine::Player();

		addEntity(Entity::Type::PlayerEntity, player);
	}
}