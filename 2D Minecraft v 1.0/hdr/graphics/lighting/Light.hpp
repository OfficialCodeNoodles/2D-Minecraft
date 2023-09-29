#pragma once

// Dependencies
#include "../../util/Loader.hpp"

namespace engine {
	using TileColor = gs::Color;

	namespace render {
		namespace lighting {
			class Light {
			public:
				enum Id {
					None,			Sunlight,
					Furnace,		Lava
				} id;

				TileColor baseColor;
				int strength; 
				int centerRadius; 
				bool flicker; 
				float flickerFrequency; 
				float flickerAmplitude; 
				bool flickerMaintainCenter; 

				Light(); 
				~Light() = default; 

				TileColor getColor(gs::Vec2i position) const; 

				static constexpr int numOfLights = 100; 
				static Light lightSources[numOfLights]; 
				static int maxLightRadius; 
			private:
			};
			
			void loadLights(); 
		}
	}
}