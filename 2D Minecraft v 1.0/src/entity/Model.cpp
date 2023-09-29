#include "../../hdr/entity/Model.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	Model::Segment::Segment() :
		textureIndex(0), 
		rotation(0.0f),
		color(gs::Color::White)
	{
	}

	Model::Model() {
	}

	Model Model::models[Model::End];
	const std::string Model::modelStrings[Model::End] = {
		"", "player", "zombie", "skeleton", "creeper", "arrow", "pig", "cow"
	}; 
	const int Model::modelLimbCount[Model::End] = {
		0, 6, 6, 6, 6, 0, 6, 6
	}; 

	ModelTransform::SegmentTransform::SegmentTransform() : 
		angle(0.0f),
		color(gs::Color::White)
	{
	}
	
	ModelTransform::ModelTransform() :
		facingForward(true), 
		animationHealth(Player::maxHealth), 
		id(Model::Id::None), 
		distanceAlongSin(0.0f)
	{
	}

	void ModelTransform::update(const Entity& entity) {
		const float walkingScaler = 0.15f / std::max(0.05f, 
			std::abs(targetVelocity.x)); 
		auto getLegWalkingAngle = [&]() -> float {
			return std::sin(distanceAlongSin) 
				* animationVelocity.x * 220.0f; 
		}; 
		auto getArmWalkingAngle = [&]() -> float {
			return std::sin(distanceAlongSin) 
				* animationVelocity.x * 150.0f; 
		}; 

		const float animationReturnPercentage = 20.0f; 
		const float scaler = facingForward ? 1.0f : -1.0f;

		distanceAlongSin += entity.velocity.x * walkingScaler;

		switch (id) {
		case Model::None:

			break; 
		case Model::Player:
		{
			const Player* playerEntity = dynamic_cast<const Player*>(&entity); 

			float frontLegAngle = getLegWalkingAngle(); 
			float backLegAngle = -frontLegAngle;
			float frontArmAngle = getArmWalkingAngle(); 
			float backArmAngle = -frontArmAngle;

			transforms[BackLeg].angle = backLegAngle;
			transforms[FrontLeg].angle = frontLegAngle;

			if (playerEntity->miningDuration != 0.0f) {
				transforms[FrontArm].angle = 40.0f + (std::sin(
					playerEntity->miningDuration / 3.0f) * 20.0f);
				transforms[FrontArm].angle *= -scaler;
			}
			else 
				gs::util::approach(&transforms[FrontArm].angle, frontArmAngle,
					animationReturnPercentage);

			gs::util::approach(&transforms[BackArm].angle, backArmAngle, 
				animationReturnPercentage);
			gs::util::approach(
				&animationHealth, static_cast<float>(playerEntity->health),
				animationReturnPercentage
			); 
		}
			break; 
		case Model::Zombie:
		{
			const Mob* mobEntity = dynamic_cast<const Mob*>(&entity); 

			float frontLegAngle = getLegWalkingAngle();
			float backLegAngle = -frontLegAngle;
			float frontArmAngle = getArmWalkingAngle();
			float backArmAngle = -frontArmAngle;

			transforms[BackLeg].angle = backLegAngle;
			transforms[FrontLeg].angle = frontLegAngle;

			const float walkingArmScaler = 1.0f
				+ (0.1f * std::sin(static_cast<float>(render::window::ticks) / 8.0f));

			transforms[FrontArm].angle = 90.0f * -scaler
				* std::min(1.0f, std::abs(animationVelocity.x * 50.0f));
			transforms[BackArm].angle = transforms[FrontArm].angle 
				* (2.0f - walkingArmScaler);
			transforms[FrontArm].angle *= walkingArmScaler; 
		}
			break; 
		case Model::Skeleton: 
		{
			const Mob* mobEntity = dynamic_cast<const Mob*>(&entity);
			
			float frontLegAngle = getLegWalkingAngle() * 0.5f;
			float backLegAngle = -frontLegAngle;
			float frontArmAngle = (90.0f + getArmWalkingAngle()) * -scaler;
			float backArmAngle = 180.0f - frontArmAngle;//-frontArmAngle;

			transforms[BackLeg].angle = backLegAngle;
			transforms[FrontLeg].angle = frontLegAngle;

			//const float walkingArmScaler = 1.0f
			//	* std::sin(static_cast<float>(render::window::ticks) / 8.0f);

			transforms[FrontArm].angle = frontArmAngle; 
			transforms[BackArm].angle = backArmAngle; 
		}
			break; 
		case Model::Creeper:
		{
			const Mob* mobEntity = dynamic_cast<const Mob*>(&entity); 

			float frontLegAngle = getLegWalkingAngle() * 5.0f;
			float backLegAngle = -frontLegAngle;

			transforms[BackLeg - 1].angle = -backLegAngle;
			transforms[BackLeg].angle = backLegAngle;
			transforms[FrontLeg].angle = -frontLegAngle;
			transforms[FrontLeg + 1].angle = frontLegAngle;
		}
			break; 
		case Model::Pig:
		{
			const Mob* mobEntity = dynamic_cast<const Mob*>(&entity); 

			float frontLegAngle = getLegWalkingAngle() * 5.0f;
			float backLegAngle = -frontLegAngle;

			transforms[BackLeg - 1].angle = -backLegAngle;
			transforms[BackLeg].angle = backLegAngle;
			transforms[FrontLeg].angle = -frontLegAngle;
			transforms[FrontLeg + 1].angle = frontLegAngle;
		}
			break; 
		case Model::Cow:
		{
			const Mob* mobEntity = dynamic_cast<const Mob*>(&entity);

			float frontLegAngle = getLegWalkingAngle() * 5.0f;
			float backLegAngle = -frontLegAngle;

			transforms[BackLeg - 1].angle = -backLegAngle;
			transforms[BackLeg].angle = backLegAngle;
			transforms[FrontLeg].angle = -frontLegAngle;
			transforms[FrontLeg + 1].angle = frontLegAngle;
		}
			break; 
		}

		// Returns segment colors back to white. 
		for (auto& transform : transforms) 
			gs::util::approach(&transform.color, gs::Color::White, 10.0f);

		// Updates generic animations. 
		gs::util::approach(&animationVelocity.x, entity.velocity.x, 
			animationReturnPercentage);
		gs::util::approach(&animationVelocity.y, entity.velocity.y, 
			animationReturnPercentage);
	}
	void ModelTransform::calculateDirection() {
		float& headAngle = transforms[Head].angle; 

		facingForward = headAngle > -90.0f && headAngle < 90.0f;
		if (!facingForward) headAngle += 180.0f;
	}
	void ModelTransform::hurt() {
		setColor(gs::Color::Red); 
	}

	void ModelTransform::setColor(gs::Color color) {
		for (auto& transform : transforms)
			transform.color = color; 
	}

	void loadModels() {
		for (int modelIndex = 0; modelIndex < Model::End; modelIndex++) {
			const std::string filename = render::assetDirectory 
				+ "data/entity/model/" + Model::modelStrings[modelIndex] 
				+ ".list";
			const PairVector& pairs = loadPairedFile(
				filename, '=', "EndProperties");

			Model& model = Model::models[modelIndex]; 
			Model::Segment* segment = nullptr; 
			int segmentIndex;

			for (auto& [attribute, value] : pairs) {
				if (attribute == "NewSegment") {
					segmentIndex = std::stoi(value); 
					segment = &model.segments[segmentIndex]; 
				}
				else if (segment != nullptr) {
					if (attribute == "textureIndex")
						segment->textureIndex = std::stoi(value);
					else if (attribute == "xpos")
						segment->position.x = std::stof(value);
					else if (attribute == "ypos")
						segment->position.y = std::stof(value);
					else if (attribute == "xOrigin")
						segment->origin.x = std::stof(value);
					else if (attribute == "yOrigin")
						segment->origin.y = std::stof(value);
					else if (attribute == "rotation")
						segment->rotation = std::stof(value);
					else if (attribute == "redValue")
						segment->color.r = std::stoi(value);
					else if (attribute == "greenValue")
						segment->color.g = std::stoi(value);
					else if (attribute == "blueValue")
						segment->color.b = std::stoi(value); 
				}
			}
		}
	}
}