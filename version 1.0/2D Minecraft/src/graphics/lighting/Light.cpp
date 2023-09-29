#include "../../../hdr/graphics/lighting/Light.hpp"
#include "../../../hdr/graphics/Render.hpp"

namespace engine {
	namespace render {
		namespace lighting {
			Light::Light() :
				id(None),
				baseColor(TileColor::White),
				strength(10),
				centerRadius(0),
				flicker(false),
				flickerFrequency(1.0f),
				flickerAmplitude(1.0f), 
				flickerMaintainCenter(false)
			{
			}

			TileColor Light::getColor(gs::Vec2i position) const {
				auto getLightPercentage = [&](float value) -> float {
					const float amplitude = 5.0f * flickerAmplitude; 
					return (amplitude * (std::sin(value) 
						+ std::cos(3.0f * value) + 0.9f)) + amplitude;
				}; 

				TileColor color = baseColor; 

				if (id == Light::Sunlight) {
					color = sunlightColor; 
					gs::util::approach(&color, moonlightColor, 
						sunlightBrightness);
				}
				
				if (flicker) {
					const float value = (static_cast<float>(
						window::ticks / 50.0f) * flickerFrequency) 
							+ position.x + (3.0f * position.y); 

					gs::util::approach(&color, TileColor::Black,
						getLightPercentage(value)); 
				}

				return color; 
			}

			Light Light::lightSources[Light::numOfLights];
			int Light::maxLightRadius = 0; 

			void loadLights() {
				const PairVector& pairs = loadPairedFile(
					render::assetDirectory + "data/light.list");

				Light::Id lightId = static_cast<Light::Id>(0);
				Light* lightSource = nullptr; 

				for (auto& [attribute, value] : pairs) {
					if (attribute == "NewLight") {
						if (lightSource != nullptr) {
							Light::maxLightRadius = std::max(
								Light::maxLightRadius, lightSource->strength 
									+ lightSource->centerRadius
							); 
						}

						lightId = static_cast<Light::Id>(std::stoi(value));
						lightSource = &Light::lightSources[lightId];
						lightSource->id = lightId;
					}
					else if (lightSource != nullptr) {
						if (attribute == "redValue")
							lightSource->baseColor.r = std::stoi(value);
						else if (attribute == "greenValue")
							lightSource->baseColor.g = std::stoi(value);
						else if (attribute == "blueValue")
							lightSource->baseColor.b = std::stoi(value);
						else if (attribute == "strength") 
							lightSource->strength = std::stoi(value);
						else if (attribute == "centerRadius")
							lightSource->centerRadius = std::stoi(value);
						else if (attribute == "flicker")
							lightSource->flicker = 
								static_cast<bool>(std::stoi(value));
						else if (attribute == "flickerFrequency")
							lightSource->flickerFrequency = std::stof(value);
						else if (attribute == "flickerAmplitude")
							lightSource->flickerAmplitude = std::stof(value);
						else if (attribute == "flickerMaintainCenter")
							lightSource->flickerMaintainCenter = 
								static_cast<bool>(std::stoi(value)); 
					} 
				}
			}
		}
	}
}