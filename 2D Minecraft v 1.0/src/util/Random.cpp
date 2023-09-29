#include "../../hdr/util/Random.hpp"

namespace engine {
	Random::Random() {
		generateSeed(); 
		setSeed(seed); 
	}
	Random::Random(int seed) : seed(seed) {
		setSeed(seed); 
	}

	void Random::generateSeed() {
		std::random_device randomDevice;

		seed = randomDevice();
	}
	int Random::generate(bool makePositive) {
		const int value = distribution(mTwister); 
		return makePositive ? std::abs(value) : value; 
	}

	void Random::setSeed(int seed) {
		this->seed = seed; 
		mTwister.seed(seed); 
	}

	int Random::getSeed() const {
		return seed;
	}

	Random randomGenerator;

	float generateNormalizedFloat(bool makePositive) {
		const int upperBound = 10000; 

		float normalizedValue = (randomGenerator.generate() % upperBound) 
			/ static_cast<float>(upperBound); 

		return makePositive ? normalizedValue : (normalizedValue * 2.0f) - 1.0f;
	}
}