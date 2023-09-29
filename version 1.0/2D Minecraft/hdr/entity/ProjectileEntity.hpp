#pragma once

// Dependencies
#include "Gravity.hpp"

namespace engine {
	class ProjectileEntity : public Entity {
	public:
		float angle; 
		float speed; 
		bool stuckInBlock;
		gs::Vec2i stuckBlockPosition; 
		int timeLeft; 

		ProjectileEntity();
		~ProjectileEntity(); 

		void update() override; 
	private:
	};
}