#include "../../hdr/world/PathFinding.hpp"

namespace engine {
	size_t HashContainer::operator()(const gs::Vec2i& vec) const {
		// Used this post as a reference to make hash for 2 component vector. 
		// https://stackoverflow.com/questions/45395071/hash-for-a-stdpair-for-use-in-an-unordered-map
		return size_t(vec.x) << 32 | vec.y;
	}

	PathFinder::PathFinder() :
		endPoint({ -1, -1 }),
		endBlockId(Block::Invalid),
		restrictions(sf::IntRect(-1, -1, -1, -1)),
		safeBlockFilter([](Block block) -> bool { return block.isEmpty(); }),
		unsafeBlockFilter([](Block block) -> bool { return false; }),
		boundsReached(false),
		endPointReached(false)
	{
	}
	PathFinder::~PathFinder() {
	}

	void PathFinder::clearRestrictions() {
		restrictions = sf::IntRect(-1, -1, -1, -1); 
	}
	bool PathFinder::isBlockLocatable(Block::Id blockId, int limit) {
		initSearch(); 
		endBlockId = blockId; 
		while (!breadthFlood() && (limit == -1 ? true 
			: searched.size() < limit));
		return endPointReached; 
	}
	int PathFinder::calculateAreaVolume(int limit) {
		initSearch(); 
		while (!breadthFlood() && searched.size() < limit); 
		return searched.size();
	}

	void PathFinder::setSafeBlockFilter(bool(*safeBlockFilter)(Block block)) {
		this->safeBlockFilter = safeBlockFilter; 
	}
	void PathFinder::setUnsafeBlockFilter(bool(*unsafeBlockFilter)(Block block)) {
		this->unsafeBlockFilter = unsafeBlockFilter; 
	}

	const std::vector<gs::Vec2i>& PathFinder::getSearchedBlocks() const {
		return searched; 
	}
	bool PathFinder::wereChunkBoundsReached() const {
		return boundsReached;
	}

	bool PathFinder::isPositionWithinBounds(gs::Vec2i position) const {
		return position.y >= 0 && position.y < Chunk::height
			&& (restrictions.left == -1 || position.x >= restrictions.left)
			&& (restrictions.top == -1 || position.y >= restrictions.top)
			&& (restrictions.width == -1 || position.x <= restrictions.width)
			&& (restrictions.height == -1 || position.y <= restrictions.height); 
	}

	void PathFinder::clearSearch() {
		while (!frontier.empty())
			frontier.pop(); 

		searched.clear(); 
		pathMap.clear(); 
		path.clear(); 
	}
	void PathFinder::initSearch() {
		clearSearch(); 

		frontier.push(startPoint); 
		searched.push_back(startPoint); 
		pathMap[startPoint] = startPoint; 
		boundsReached = false; 
		endPointReached = false; 
	}

	bool PathFinder::breadthFlood() {
		if (!frontier.empty()) {
			gs::Vec2i currentPosition = frontier.front(); 
			frontier.pop();

			if (currentPosition == endPoint
				|| world->getBlock(currentPosition).id == endBlockId)
			{
				endPointReached = true; 
				return true;
			}

			gs::Vec2i neighborPositions[4]; 
			int validNeighbors = 0; 

			for (int neighborIndex = 0; neighborIndex < 4; neighborIndex++) {
				gs::Vec2i neighborPosition = currentPosition;

				if (neighborIndex % 2 == 0)
					neighborPosition.y += neighborIndex == 0 ? -1 : 1;
				else
					neighborPosition.x += neighborIndex == 1 ? 1 : -1;

				if (world->getChunk(world->getChunkOffset(neighborPosition.x)) 
						== nullptr)
					boundsReached = true; 
				else if (isPositionWithinBounds(neighborPosition)) {
					const Block neighborBlock = 
						world->getBlock(neighborPosition); 

					if (unsafeBlockFilter(neighborBlock)) {
						clearSearch(); 
						return true; 
					}

					if (safeBlockFilter(neighborBlock)) {
						neighborPositions[validNeighbors] = neighborPosition;
						validNeighbors++;
					}
				}

				for (int nextPositionIndex = 0; nextPositionIndex <
					validNeighbors; nextPositionIndex++)
				{
					const gs::Vec2i nextPosition = 
						neighborPositions[nextPositionIndex]; 

					if (std::find(searched.begin(), searched.end(),
						nextPosition) == searched.end())
					{
						frontier.push(nextPosition); 
						searched.push_back(nextPosition); 
						pathMap[nextPosition] = currentPosition; 
					}
				}
			}

			return false; 
		}

		return true; 
	}
}