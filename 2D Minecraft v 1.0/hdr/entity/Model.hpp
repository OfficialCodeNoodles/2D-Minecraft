#pragma once

// Dependencies
#include "../util/Loader.hpp"

namespace engine {
	class Entity; 

	class Model {
	public:
		enum Id { 
			None, Player, Zombie, Skeleton, Creeper, Arrow, Pig, Cow, End 
		}; 
		struct Segment {
			int textureIndex; 
			gs::Vec2f position; 
			gs::Vec2f origin; 
			float rotation; 
			gs::Color color; 

			Segment(); 
			~Segment() = default; 

			static constexpr int maxNumOfSegments = 10; 
		};

		Segment segments[Segment::maxNumOfSegments]; 

		Model(); 
		~Model() = default; 

		static Model models[Model::End]; 
		static const std::string modelStrings[End];
		static const int modelLimbCount[End]; 
	private:
	};

	class ModelTransform {
	public:
		enum Segments { BackArm, BackLeg, Torso, Head, FrontLeg, FrontArm };
		struct SegmentTransform {
			float angle; 
			gs::Color color; 

			SegmentTransform(); 
			~SegmentTransform() = default; 
		};

		SegmentTransform transforms[Model::Segment::maxNumOfSegments]; 
		gs::Vec2f animationVelocity; 
		gs::Vec2f targetVelocity; 
		bool facingForward; 
		float animationHealth; 
		Model::Id id; 

		ModelTransform();
		~ModelTransform() = default; 

		void update(const Entity& entity); 
		void calculateDirection(); 
		void hurt();  

		void setColor(gs::Color color); 
	private:
		float distanceAlongSin; 
	};

	void loadModels(); 
}

#include "../entity/Entity.hpp"