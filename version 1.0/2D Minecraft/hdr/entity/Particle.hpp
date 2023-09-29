#pragma once

// Dependencies
#include "../Resources.hpp"

namespace engine {
	class Particle {
	public:
		enum class Type { Generic, Star, DamagePoint, End } type;
		gs::Vec2f position; 
		gs::Vec2f velocity; 
		gs::Vec2f size; 
		float mass; 
		bool collisionEnabled; 
		float rotation; 
		float rotationalSpeed; 
		gs::Color color; 
		std::string string; 
		int timeRemaining; 
		bool active; 
		bool reserved; 

		Particle(); 
		Particle(Type type); 
		~Particle() = default; 

		void applyAcceleration(gs::Vec2f acceleration); 
		void update(); 

		static constexpr int numOfParticles = 1000; 
		static const bool litParticles[static_cast<int>(Type::End)]; 
		static const int numOfStars; 
		static const float starLowerHorizon; 

		static void generateBlockParticles(
			gs::Vec2i blockPosition, bool collisionEnabled = false
		); 
		static void generateWallParticles(
			gs::Vec2i wallPosition, bool collisionEnabled = false
		); 
	private:
		void updatePosition(); 

		static void generateTileParticles(
			gs::Vec2i tilePosition, bool isBlock, bool collisionEnabled
		); 
	};

	extern Particle particles[Particle::numOfParticles]; 
	extern int particlesRendered; 

	void initStars(); 
	void addParticle(const Particle& particle); 
	void updateParticles(); 
}