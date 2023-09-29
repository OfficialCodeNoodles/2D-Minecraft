#include "../../hdr/entity/Particle.hpp"
#include "../../hdr/graphics/Window.hpp"
#include "../../hdr/util/Random.hpp"
#include "../../hdr/entity/Gravity.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	Particle::Particle() :
		type(Type::Generic),
		mass(1.0f), 
		collisionEnabled(false), 
		rotation(0.0f),
		rotationalSpeed(0.0f),
		timeRemaining(-1), 
		active(false),
		reserved(false)
	{
	}
	Particle::Particle(Type type) : Particle() {
		this->type = type; 

		switch (type) {
		case Type::Generic:
			size.x = 1.0f / 12.0f; 
			size.y = 1.0f / 12.0f; 
			rotation = 0.0f; 
			rotationalSpeed = 0.0f; 
			break; 
		case Type::Star:
		{
			const float maxStarSize = 6.0f;
			const float minStarSize = 1.0f; 

			position.x = generateNormalizedFloat() 
				* static_cast<float>(render::window::defaultWindowWidth); 
			position.y = generateNormalizedFloat() * starLowerHorizon;
			velocity.x = generateNormalizedFloat(false) * 0.1f; 
			velocity.y = generateNormalizedFloat(false) * 0.1f; 
			size.x = minStarSize + (generateNormalizedFloat()
				* (maxStarSize - minStarSize)); 
			size.y = size.x; 
			rotation = generateNormalizedFloat() * 360.0f; 
			rotationalSpeed = generateNormalizedFloat(false) * 3.0f; 
			color = gs::Color(
				220, 230, 255, 100.0f + (generateNormalizedFloat() * 100.0f)
			); 
			reserved = true; 
		}
			break; 
		case Type::DamagePoint:
			timeRemaining = 60; 
			color = gs::Color::Yellow; 
			break;
		}

		mass = 1.0f; 
		collisionEnabled = false; 
		// If type is passed into the constructor, it is assumed to be needed.
		active = true; 
	}
	
	void Particle::applyAcceleration(gs::Vec2f acceleration) {
		velocity += acceleration; 
	}
	void Particle::update() {
		switch (type) {
		case Type::Generic:
			applyGravity(*this); 
			
			gs::util::approach(
				&velocity.x, 0.0f, collisionEnabled ? 5.0f : 20.0f
			); 

			color.a = std::min(
				timeRemaining * (collisionEnabled ? 5 : 15), 255
			); 
			break;
		case Type::Star:
		{
			const float verticalSafetyOffset = 10.0f; 

			position.x = gs::util::mod(
				position.x,
				static_cast<float>(render::window::defaultWindowWidth)
			);
			position.y += verticalSafetyOffset;
			position.y = gs::util::mod(position.y, starLowerHorizon);
			position.y -= verticalSafetyOffset;
		}
			break;
		case Type::DamagePoint:
			break; 
		}

		if (collisionEnabled)
			collision::applyCollision(*this, *world);

		updatePosition();
		timeRemaining = std::max(timeRemaining - 1, -1); 
		// Stops using particle if the time remaining equals zero. 
		active = timeRemaining != 0; 
	}

	const bool Particle::litParticles[static_cast<int>(Type::End)] = {
		true, false, false 
	};
	const int Particle::numOfStars = 200;
	const float Particle::starLowerHorizon = 600.0f;
	
	void Particle::generateBlockParticles(
		gs::Vec2i blockPosition, bool collisionEnabled) 
	{
		generateTileParticles(blockPosition, true, collisionEnabled); 
	}
	void Particle::generateWallParticles(
		gs::Vec2i wallPosition, bool collisionEnabled) 
	{
		generateTileParticles(wallPosition, false, collisionEnabled); 
	}

	void Particle::updatePosition() {
		position += velocity; 
		rotation += rotationalSpeed; 
	}

	void Particle::generateTileParticles(
		gs::Vec2i tilePosition, bool isBlock, bool collisionEnabled) 
	{
		const int numOfTileParticlesToGenerate = collisionEnabled ? 10 : 1;
		const int tileId = isBlock ? world->getBlockId(tilePosition)
			: world->getWallId(tilePosition); 

		if (tileId <= 0 || (isBlock && Block(static_cast<Block::Id>(tileId)).isFluid()))
			return; 

		for (int tileParticleIndex = 0; tileParticleIndex <
			numOfTileParticlesToGenerate; tileParticleIndex++)
		{
			Particle tileParticle(Particle::Type::Generic); 

			tileParticle.position = gs::Vec2f(
				tilePosition.x + 0.5f + (static_cast<float>(
					randomGenerator.generate(false) % 100) / 500.0f),
				tilePosition.y + 0.5f + (static_cast<float>(
					randomGenerator.generate(false) % 100) / 500.0f)
			); 
			tileParticle.velocity = gs::Vec2f(
				static_cast<float>(randomGenerator.generate(false) % 100)
					/ 1000.0f,
				static_cast<float>(randomGenerator.generate(false) % 100)
					/ 1000.0f
			);
			tileParticle.mass = collisionEnabled ? 0.5f : 0.15f; 
			tileParticle.collisionEnabled = collisionEnabled;
			tileParticle.color = gs::util::approach(
				isBlock ? render::getBlockPixelColor(static_cast<Block::Id>(tileId)) 
					: render::getWallPixelColor(static_cast<Wall::Id>(tileId)),
				gs::Color::Black, 10.0f
			);
			tileParticle.timeRemaining = collisionEnabled ? 300 : 20; 

			addParticle(tileParticle); 
		}
	}

	Particle particles[Particle::numOfParticles];
	int particlesRendered = 0; 
	
	void initStars() {
		for (int starIndex = 0; starIndex < Particle::numOfStars; starIndex++) 
			addParticle(Particle(Particle::Type::Star)); 
	}
	void addParticle(const Particle& particle) {
		for (auto& oldParticle : particles) {
			if (!oldParticle.reserved && !oldParticle.active) {
				oldParticle = particle; 
				oldParticle.active = true; 
				break; 
			}
		}
	}
	void updateParticles() {
		if (world == nullptr)
			return; 

		for (int particleIndex = 0; particleIndex < Particle::numOfParticles;
			particleIndex++)
		{
			Particle& particle = particles[particleIndex]; 

			if (particle.active)
				particle.update(); 
		}
	}
}