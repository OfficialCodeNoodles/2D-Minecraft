#include "../hdr/Game.hpp"

// Globals

gs::util::State menuState;
GameMode gameMode = GameMode::Survival; 

namespace game {
	const std::string gamePropertiesFileName = "gameProperties.sav";

	gs::util::Clock timer;

	void create() {
#ifndef mDebug
		FreeConsole(); 
#endif
		srand(time(0));
		menuState.setState(static_cast<int>(MenuState::LoadingScreen));
		engine::render::window::open(true);
		engine::input::initKeys();
		loadGameProperties(); 
		engine::render::loadAssets();
		engine::render::ui::loadAssets();
		engine::render::lighting::loadLights();
		engine::loadBlockInfo();
		engine::loadWallInfo();
		engine::loadBiomeInfo();
		engine::loadItemInfo();
		engine::loadRecipes(); 
		engine::loadModels();
		engine::loadLootTables();
		engine::loadSpawnInfo();
		engine::loadDefaultGenerators();
		engine::loadWorldPreviews(); 
		engine::audio::loadMusicTracks(); 
		engine::audio::loadSoundEffects(); 
		engine::initStars();
		engine::createPlayer(); 
	}
	void update() {
		timer.begin(); 

		updateEvents();

		if (!isOpen())
			return;

		const int frameRateDivisor = engine::render::window::framerate
			/ engine::render::window::renderingFramerate;

		if (engine::render::window::ticks % frameRateDivisor == 0)
			updateGraphics(); 

		timer.end(); 
		engine::render::window::currentUncappedFramerate = 
			timer.getUncappedFrameRate(); 
		 
		timer.wait(engine::render::window::framerate); 
		engine::render::window::currentFramerate = timer.getFrameRate()
			/ frameRateDivisor; 
	}
	void close() {
		saveGameProperties(); 

		if (engine::world != nullptr)
			engine::world->saveWorld(); 

		delete engine::world; 
		engine::deleteEntities();

		engine::render::window::close(); 
	}
	bool isOpen() {
		return engine::render::window::winmain->isOpen(); 
	}
	void loadGameProperties() {
		const engine::PairVector& pairs = 
			engine::loadPairedFile(gamePropertiesFileName);

		for (auto& [attribute, value] : pairs) {
			if (attribute == "musicVolume")
				engine::audio::adjustMusicVolume(std::stof(value));
			else if (attribute == "soundVolume")
				engine::audio::adjustSoundVolume(std::stof(value));
			else if (attribute == "framerate")
				engine::render::window::renderingFramerate = std::stoi(value);
			else if (attribute == "lightingStyle")
				engine::render::lighting::lightingStyle =
					static_cast<engine::render::lighting::LightingStyle>(
						std::stoi(value)
					);
			else if (attribute == "renderBiomeBackground")
				engine::render::shouldBiomeBackgroundBeRendered =
					static_cast<bool>(std::stoi(value));
			else if (attribute == "renderSunAndMoon")
				engine::render::shouldSunAndMoonBeRendered =
					static_cast<bool>(std::stoi(value));
			else if (attribute == "renderParticles")
				engine::render::shouldParticlesBeRendered = 
					static_cast<bool>(std::stoi(value));
			else if (attribute == "renderStars")
				engine::render::shouldStarsBeRendered =
					static_cast<bool>(std::stoi(value));
			else if (attribute == "keyMoveLeft")
				engine::input::keyMap[engine::input::MoveLeft] = 
					static_cast<sf::Keyboard::Key>(std::stoi(value)); 
			else if (attribute == "keyMoveRight")
				engine::input::keyMap[engine::input::MoveRight] =
					static_cast<sf::Keyboard::Key>(std::stoi(value));
			else if (attribute == "keyJump")
				engine::input::keyMap[engine::input::Jump] =
					static_cast<sf::Keyboard::Key>(std::stoi(value));
			else if (attribute == "OpenInventory")
				engine::input::keyMap[engine::input::OpenInventory] = 
					static_cast<sf::Keyboard::Key>(std::stoi(value));
		}	
	}
	void saveGameProperties() {
		engine::PairVector pairs;

		// Adds game properties to the save file. 
		pairs.emplace_back("musicVolume", toString(engine::audio::musicVolume));
		pairs.emplace_back("soundVolume", toString(engine::audio::soundVolume));
		pairs.emplace_back(
			"framerate", toString(engine::render::window::renderingFramerate)
		); 
		pairs.emplace_back(
			"lightingStyle", toString(static_cast<int>(
				engine::render::lighting::lightingStyle))
		); 
		pairs.emplace_back(
			"renderBiomeBackground", toString(static_cast<int>(
				engine::render::shouldBiomeBackgroundBeRendered))
		); 
		pairs.emplace_back(
			"renderSunAndMoon", toString(static_cast<int>(
				engine::render::shouldSunAndMoonBeRendered))
		); 
		pairs.emplace_back(
			"renderParticles", toString(static_cast<int>(
				engine::render::shouldParticlesBeRendered))
		); 
		pairs.emplace_back(
			"renderStars", toString(static_cast<int>(
				engine::render::shouldStarsBeRendered))
		); 
		pairs.emplace_back(
			"keyMoveLeft", toString(static_cast<int>(
				engine::input::keyMap[engine::input::MoveLeft]))
		); 
		pairs.emplace_back(
			"keyMoveRight", toString(static_cast<int>(
				engine::input::keyMap[engine::input::MoveRight]))
		);
		pairs.emplace_back(
			"keyJump", toString(static_cast<int>(
				engine::input::keyMap[engine::input::Jump]))
		);
		pairs.emplace_back(
			"keyOpenInventory", toString(static_cast<int>(
				engine::input::keyMap[engine::input::OpenInventory]))
		);

		engine::savePairedFile(gamePropertiesFileName, pairs);
	}

	void updateEvents() {
		engine::render::window::update(); 

		if (!isOpen())
			return; 

		// Exits function to pause updates when the window isn't focused on. 
		if (!engine::render::window::winmain->hasFocus()) {
			engine::audio::pauseMusicTrack(); 
			engine::render::window::ticks--; 
			return; 
		}

		engine::input::updateKeys(); 
		engine::audio::pauseMusicTrack(false); 
		
		switch (static_cast<MenuState>(menuState.getState())) {
		case MenuState::LoadingScreen:
			engine::render::ui::updateLoadingScreen(); 
			engine::loadResource(); 
			break; 
		case MenuState::TitleScreen:
			engine::render::ui::updateLoadingScreen(); 
			engine::render::ui::updateTitleScreen(); 
			engine::updateParticles();
			break; 
		case MenuState::WorldSelection:
			engine::render::ui::updateWorldSelectionScreen(); 
			break; 
		case MenuState::WorldEdit:
			engine::render::ui::updateWorldEditScreen(); 
			break; 
		case MenuState::WorldCreate:
			engine::render::ui::updateWorldCreateScreen(); 
			break; 
		case MenuState::ChangeLog:
			engine::render::ui::updateChangeLogScreen(); 
			break; 
		case MenuState::Credits:
			engine::render::ui::updateCreditsScreen(); 
			break; 
		case MenuState::Settings:
			engine::render::ui::updateSettingsScreen(); 
			break; 
		case MenuState::InGame:
			// Update world
			if (!engine::render::ui::gamePaused) {
				engine::updateMobSpawning();
				engine::updateEntities();
				engine::updateParticles(); 
				engine::render::updateCamera();
				engine::world->update();
				engine::render::updateBackground();
				engine::render::lighting::updateWorldLights(*engine::world);
				engine::updateMobSpawning(); 
				engine::audio::updateAudioEnviroment(); 
			}
			// Update UI
			engine::render::ui::updateIngameUI();
			engine::render::ui::updateCrosshair(); 
			engine::render::transition.update(); 
			break; 
		}
	}
	void updateGraphics() {
		engine::render::window::begin();

		switch (static_cast<MenuState>(menuState.getState())) {
		case MenuState::LoadingScreen:
			engine::render::ui::renderLoadingScreen(); 
			break; 
		case MenuState::TitleScreen:
			engine::render::renderSky(engine::render::ui::titleGameTime);
			engine::render::renderStars(engine::render::ui::titleGameTime);
			engine::render::renderSunAndMoon(engine::render::ui::titleGameTime); 
			engine::render::ui::renderTitleScreen(); 
			engine::render::ui::renderLoadingScreen(); 
			break; 
		case MenuState::WorldSelection:
			engine::render::ui::renderWorldSelectionScreen(); 
			break; 
		case MenuState::WorldEdit:
			engine::render::ui::renderWorldEditScreen(); 
			break; 
		case MenuState::WorldCreate:
			engine::render::ui::renderWorldCreateScreen(); 
			break; 
		case MenuState::ChangeLog:
			engine::render::ui::renderChangeLogScreen(); 
			break; 
		case MenuState::Credits:
			engine::render::ui::renderCreditsScreen(); 
			break; 
		case MenuState::Settings:
			engine::render::ui::renderSettingsScreen(); 
			break; 
		case MenuState::InGame:
			// Render world 
			engine::render::renderBackground();
			engine::render::renderWalls(*engine::world);
			engine::render::renderBlocks(*engine::world);
			engine::render::renderLitParticles(); 
			engine::render::ui::renderTileSelectionOutline(); 
			engine::render::renderEntities();
			engine::render::renderFluids(*engine::world);
			engine::render::renderWorldLightMap(*engine::world);
			engine::render::finishWorldRendering();
			engine::render::renderUnlitParticles(); 
			// Render UI
			engine::render::ui::renderIngameUI();
			engine::render::ui::renderCrosshair();
			engine::render::transition.apply(*engine::render::window::winmain);  
			break; 
		}

		engine::render::window::end(); 
	}
}