#pragma once

// Dependencies
#include "../Resources.hpp"

namespace engine {
	using StringPair = std::pair<std::string, std::string>;
	using PairVector = std::vector<StringPair>; 

	StringPair seperate(const std::string& string, char seperator = '='); 

	const PairVector& loadPairedFile(
		const std::string& filename, char seperator = '=',
		const std::string& end = "End", bool removeSpaces = true
	);
	void savePairedFile(
		const std::string& filename, const PairVector& pairs, 
		char seperator = '='
	); 

	struct ResourceLoader {
		enum class Type { Texture, Font, MusicTrack, SoundEffect } type;
		void* resource; 
		sf::Sprite* sprite; 
		sf::Sound* sound; 
		std::string fileName; 

		ResourceLoader();
		ResourceLoader(Type type, const std::string& fileName, void* resource); 
		ResourceLoader(
			Type type, const std::string& fileName, void* resource, 
			sf::Sprite* sprite
		); 
		ResourceLoader(
			Type type, const std::string& fileName, void* resource,
			sf::Sound* sound
		);
		~ResourceLoader() = default; 
	};

	extern std::vector<ResourceLoader> resourceLoaders; 
	extern int highestNumOfResources; 

	void addResourceLoader(const ResourceLoader& resourceLoader); 
	bool loadResource(); 
	void loadResources();

	struct WorldPreview {
		std::string folderName, worldName; 
		float versionNumber; 
		sf::Texture* icon; 
		  
		WorldPreview();
		~WorldPreview(); 
	};

	extern std::vector<std::unique_ptr<WorldPreview>> worldPreviews; 

	void loadWorldPreview(const std::string& folderName); 
	void loadWorldPreviews(); 
}