#pragma once

// Dependencies
#include "Entity.hpp"

namespace engine {
	constexpr float gravitationalStrength = 0.025f; 
	constexpr float terminalVelocity = 0.5f; 

	void applyGravity(Entity& entity); 
	void applyGravity(Particle& particle); 
}