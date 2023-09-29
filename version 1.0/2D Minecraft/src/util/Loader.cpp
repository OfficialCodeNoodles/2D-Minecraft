#include "../../hdr/util/Loader.hpp"

namespace engine {
	StringPair seperate(const std::string& string, char seperator) {
		const size_t seperatorIndex = string.find(seperator); 
		return seperatorIndex == std::string::npos ? StringPair(string, "")
			: StringPair(
				string.substr(0, seperatorIndex),
				string.substr(seperatorIndex + 1)
			);
	}

	const PairVector& loadPairedFile(
		const std::string& filename, char seperator, const std::string& end, 
		bool removeSpaces)
	{
		static PairVector pairs; 

		std::ifstream ifile(filename);
		std::string line; 

		pairs.clear(); 

		while (std::getline(ifile, line)) {
			std::string trimmedLine; 
			bool seperatorFound = false;
			bool pastSeperatorGap = false;

			if ((line.size() > 0 && std::isdigit(line[0])) || !removeSpaces)
				trimmedLine = line; 
			else {
				for (char chr : line) {
					if (chr == seperator)
						seperatorFound = true;
					else if (seperatorFound && chr != ' ')
						pastSeperatorGap = true;

					if (!pastSeperatorGap && chr != ' ')
						trimmedLine += chr;
					else if (pastSeperatorGap)
						trimmedLine += chr;
				}
			}

			StringPair pair = seperate(trimmedLine, seperator);

			if (pair.first == end)
				break; 

			pairs.push_back(pair); 
		}

		ifile.close(); 

		return pairs; 
	}
	void savePairedFile(
		const std::string& filename, const PairVector& pairs, char seperator) 
	{
		std::ofstream ofile(filename); 
		
		ofile.clear(); 

		for (auto& [attribute, value] : pairs) 
			ofile << attribute << " " << seperator << " " << value << "\n";

		ofile.close(); 
	}

	ResourceLoader::ResourceLoader() : 
		resource(nullptr),
		sprite(nullptr),
		sound(nullptr)
	{
	}
	ResourceLoader::ResourceLoader(
			Type type, const std::string& fileName, void* resource) :
		type(type),
		resource(resource),
		sprite(nullptr),
		sound(nullptr),
		fileName(fileName)
	{
	}
	ResourceLoader::ResourceLoader(
		Type type, const std::string& fileName, void* resource,
			sf::Sprite* sprite) :
		type(type),
		resource(resource),
		sprite(sprite),
		sound(nullptr),
		fileName(fileName)
	{
	}
	ResourceLoader::ResourceLoader(
		Type type, const std::string& fileName, void* resource,
			sf::Sound* sound) :
		type(type),
		resource(resource),
		sprite(nullptr), 
		sound(sound),
		fileName(fileName)
	{
	}

	std::vector<ResourceLoader> resourceLoaders;
	int highestNumOfResources = 0; 

	void addResourceLoader(const ResourceLoader& resourceLoader) {
		resourceLoaders.push_back(resourceLoader); 
		highestNumOfResources = std::max(highestNumOfResources, 
			static_cast<int>(resourceLoaders.size())); 
	}
	bool loadResource() {
		if (!resourceLoaders.empty()) {
			const ResourceLoader& resourceLoader = resourceLoaders.back(); 

			switch (resourceLoader.type) {
			case ResourceLoader::Type::Texture:
			{
				sf::Texture* texture = reinterpret_cast<sf::Texture*>(
					resourceLoader.resource
				);

				if (texture->loadFromFile(resourceLoader.fileName)) {
					if (resourceLoader.sprite != nullptr)
						resourceLoader.sprite->setTexture(*texture, true);
				}
			}
				break; 
			case ResourceLoader::Type::Font:
			{
				sf::Font* font = reinterpret_cast<sf::Font*>(
					resourceLoader.resource
				); 

				font->loadFromFile(resourceLoader.fileName); 
			}
				break; 
			case ResourceLoader::Type::MusicTrack:
			{
				sf::Music* musicTrack = reinterpret_cast<sf::Music*>(
					resourceLoader.resource
				);

				musicTrack->openFromFile(resourceLoader.fileName); 
			}
				break; 
			case ResourceLoader::Type::SoundEffect:
			{
				sf::SoundBuffer* soundBuffer =
					reinterpret_cast<sf::SoundBuffer*>(
						resourceLoader.resource
					);

				if (soundBuffer->loadFromFile(resourceLoader.fileName)) {
					if (resourceLoader.sound != nullptr)
						resourceLoader.sound->setBuffer(*soundBuffer); 
				}
			}
				break; 
			}

			resourceLoaders.pop_back(); 
		}

		return resourceLoaders.empty();
	}
	void loadResources() {
		while (!loadResource()); 
	}

	WorldPreview::WorldPreview() : 
		versionNumber(-1.0f),
		icon(nullptr) 
	{
	}
	WorldPreview::~WorldPreview() {
		delete icon; 
	}

	std::vector<std::unique_ptr<WorldPreview>> worldPreviews;

	void loadWorldPreview(const std::string& folderName) {
		worldPreviews.push_back(std::make_unique<WorldPreview>()); 

		WorldPreview& worldPreview = *worldPreviews.back(); 

		worldPreview.folderName = seperate(folderName, '/').second;

		const PairVector& pairs = loadPairedFile(folderName + "/world.sav");

		// Reads properties from save file. 
		for (auto& [attribute, value] : pairs) {
			if (attribute == "name")
				worldPreview.worldName = value;
			else if (attribute == "versionNumber")
				worldPreview.versionNumber = std::stof(value); 
		}

		// Load in icon.

		const std::string iconFilename = folderName + "/icon.png"; 

		if (std::filesystem::exists(iconFilename)) {
			worldPreview.icon = new sf::Texture(); 
			worldPreview.icon->loadFromFile(iconFilename);
		}
	}
	void loadWorldPreviews() {
		const std::string& savesDirectory = "saves/"; 

		worldPreviews.clear(); 

		for (auto& entry : std::filesystem::directory_iterator(savesDirectory)) 
			loadWorldPreview(entry.path().string()); 
	}
}