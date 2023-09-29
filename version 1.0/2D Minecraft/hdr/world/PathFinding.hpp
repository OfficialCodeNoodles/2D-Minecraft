#pragma once

// Dependencies
#include <queue>
#include <unordered_map>

#include "World.hpp"

namespace engine {
	struct HashContainer {
		size_t operator()(const gs::Vec2i& vec) const; 
	};

	class PathFinder {
	public:
		gs::Vec2i startPoint, endPoint; 
		Block::Id endBlockId; 
		sf::IntRect restrictions; 
		std::vector<gs::Vec2i> path; 

		PathFinder();
		~PathFinder(); 

		void clearRestrictions(); 
		bool isBlockLocatable(Block::Id blockId, int limit = -1); 
		int calculateAreaVolume(int limit); 

		void setSafeBlockFilter(bool(*safeBlockFilter)(Block block)); 
		void setUnsafeBlockFilter(bool(*unsafeBlockFilter)(Block block)); 

		const std::vector<gs::Vec2i>& getSearchedBlocks() const; 
		bool wereChunkBoundsReached() const; 
	private:
		std::queue<gs::Vec2i> frontier; 
		std::vector<gs::Vec2i> searched; 
		std::unordered_map<gs::Vec2i, gs::Vec2i, HashContainer> pathMap; 
		bool(*safeBlockFilter)(Block block);
		bool(*unsafeBlockFilter)(Block block); 
		bool boundsReached; 
		bool endPointReached; 

		bool isPositionWithinBounds(gs::Vec2i position) const; 

		void clearSearch(); 
		void initSearch(); 
		bool breadthFlood(); 
	};
}