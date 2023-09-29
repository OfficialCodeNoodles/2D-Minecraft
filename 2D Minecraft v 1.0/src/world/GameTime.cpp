#include "../../hdr/world/GameTime.hpp"

namespace engine {
	GameTime::GameTime() : gameTicks(0) {
	}
	GameTime::GameTime(GameTick gameTicks) : gameTicks(gameTicks) {
	}

	void GameTime::tick(GameTick tickRate, bool overflowAllowed) {
		gameTicks += tickRate; 

		if (overflowAllowed)
			gameTicks %= maxTime; 
	}

	const TimeStruct getCurrentTime() {
		const std::time_t currentTime = std::time(0); 
		TimeStruct timeStruct; 
		::localtime_s(&timeStruct, &currentTime);
		return timeStruct; 
	}
}