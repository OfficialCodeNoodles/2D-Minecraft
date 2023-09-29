#pragma once

// Dependencies
#include "../Resources.hpp"

namespace engine {
	class Random {
	public:
		Random();
		Random(int seed);
		~Random() = default; 

		void generateSeed(); 
		int generate(bool makePositive = true); 

		void setSeed(int seed); 

		int getSeed() const; 
	private:
		int seed; 
		std::mt19937 mTwister;
		std::uniform_int_distribution<std::mt19937::result_type> distribution; 
	};

	extern Random randomGenerator; 

	float generateNormalizedFloat(bool makePositive = true);
}