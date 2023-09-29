#include "../../hdr/entity/Gravity.hpp"

namespace engine {
	void applyGravity(Entity& entity) {
		gs::Vec2f& velocity = entity.velocity;

		velocity.y += gravitationalStrength; 
		velocity.y = std::min(velocity.y, terminalVelocity * entity.mass); 
	}
	void applyGravity(Particle& particle) {
		gs::Vec2f& velocity = particle.velocity; 

		velocity.y += gravitationalStrength; 
		velocity.y = std::min(velocity.y, terminalVelocity * particle.mass); 
	}
}