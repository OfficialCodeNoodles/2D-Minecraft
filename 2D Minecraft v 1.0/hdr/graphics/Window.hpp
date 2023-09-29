#pragma once

// Dependencies
#include "../Resources.hpp"

namespace engine {
	namespace render {
		namespace window {
			constexpr float defaultWindowWidth = 1920.0f; 
			constexpr float defaultWindowHeight = 1080.0f; 
			extern const float defaultAspectRatio; 
			extern const std::string gameTitle; 
			extern const gs::Vec2u defaultWindowSize; 
			extern const gs::Vec2f screenCenter; 
			extern const int framerate; 

			extern sf::RenderWindow* winmain; 
			extern bool isFullScreen;
			extern sf::Image icon; 
			extern int renderingFramerate; 
			extern int currentFramerate; 
			extern int currentUncappedFramerate; 
			extern int ticks; 
			extern int mouseDelta; 

			gs::Vec2u getDesktopWindowSize(); 

			void open(bool fullscreen = false); 
			void initWindowStates(); 
			void update(); 
			void close(); 

			void begin(gs::Color fillColor = gs::Color()); 
			void end(); 
		}
	}
}