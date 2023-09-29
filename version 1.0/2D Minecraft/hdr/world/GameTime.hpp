#pragma once

// Dependencies
#include "../Resources.hpp"

namespace engine {
	using TimeStruct = std::tm;

	struct GameTime {
		using GameTick = int; 
		GameTick gameTicks; 

		GameTime(); 
		GameTime(GameTick gameTicks); 
		~GameTime() = default; 

		void tick(GameTick tickRate = 20, bool overflowAllowed = true); 

		static constexpr int maxTime = 24000; 
	};

	const TimeStruct getCurrentTime(); 
}