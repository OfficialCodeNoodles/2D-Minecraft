#pragma once

// Dependencies
#include "Window.hpp"
#include "../world/World.hpp"
#include "../entity/Player.hpp"
#include "../entity/Mob.hpp"
#include "../entity/ItemEntity.hpp"
#include "../entity/ProjectileEntity.hpp"
#include "../entity/Particle.hpp"
#include "lighting/Lighting.hpp"

namespace engine {
	namespace render {
		constexpr float tileSize = 16.0f; 
		constexpr float maxCameraScale = 10.0f; 
		constexpr float minCameraScale = 1.0f; 
		const std::string assetDirectory = "assets/";

		extern gs::Vec2f normalizedCameraPosition;
		extern gs::Vec2f cameraPosition; 
		extern gs::Vec2i cameraClipingRange; 
		extern float cameraScale; 
		extern gs::Vec2i renderableChunkRange; 
		extern gs::Vec2i renderableVerticalRange; 
		extern int minimumLightDistance; 
		extern gs::Vec2i renderableHorizontalLightRange; 
		extern gs::Vec2i renderableVerticalLightRange; 
		extern gs::Vec2i renderableHorizontalSpawningLightRange; 
		extern gs::Vec2i renderableVerticalSpawningLightRange; 

		void updateCamera(); 
		float scaleValue(float value); 
		gs::Vec2f scalePosition(gs::Vec2f position); 
		gs::Vec2f transformPosition(gs::Vec2f position);
		gs::Vec2f transformTilePosition(
			gs::Vec2i chunkPosition, int chunkOffset
		);
		const sf::Image& takeScreenshot(); 

		constexpr int maxNumOfTiles = 15000; 

		extern gs::Transition transition; 
		extern sf::VertexArray skyBox; 
		extern sf::RenderTexture worldRenderTexture; 
		extern sf::Sprite worldRenderTextureSprite; 
		extern bool shouldSunAndMoonBeRendered; 
		extern sf::Texture sunTexture, moonTextures;
		extern sf::Sprite sunSprite, moonSprite; 
		extern bool shouldBiomeBackgroundBeRendered; 
		extern std::pair<std::pair<Biome::Id, int>, std::pair<Biome::Id, int>> 
			biomeBackgrounds; 
		extern Biome::Id prvsBiome; 
		extern sf::Texture biomeBackgroundTextures[Biome::End]; 
		extern sf::Sprite biomeBackgroundSprite; 
		extern bool shouldParticlesBeRendered; 
		extern bool shouldStarsBeRendered; 
		extern float starBrightness; 
		extern sf::Texture blockAtlas, wallAtlas;
		extern sf::Image blockAtlasImage, wallAtlasImage; 
		extern sf::Texture blockOverlayAtlas; 
		extern sf::Sprite blockSprite, wallSprite; 
		extern sf::Vertex tileVerticies[maxNumOfTiles * 4]; 
		extern sf::Vertex tileOverlayVerticies[maxNumOfTiles * 4]; 
		extern int tileVertexIndex, tileOverlayVertexIndex; 
		extern sf::RenderTexture lightMap; 
		extern sf::Sprite lightMapSprite; 
		extern sf::Vertex lightMapVerticies[maxNumOfTiles * 4]; 
		extern int lightVertexIndex; 
		extern sf::Texture tileBreakingStageTextures[10]; 
		extern sf::Sprite tileBreakingStageSprite; 
		extern int tilesRendered; 
		extern sf::Texture entitySegmentTextures[Model::End]
			[Model::Segment::maxNumOfSegments];
		extern sf::Sprite entitySegmentSprite; 

		void loadAssets(); 
		void handleAssets(); 
		float getBaseSkyBrightness(GameTime::GameTick gameTicks); 
		int getBlockTextureIndex(Block block); 
		int getWallTextureIndex(Wall wall); 
		gs::Color getTilePixelColor(int textureIndex, const sf::Image& tileAtlasImage); 
		gs::Color getBlockPixelColor(Block::Id blockId);
		gs::Color getWallPixelColor(Wall::Id wallId); 
		sf::IntRect generateTextureBounds(int textureIndex);
		void applyTextureBounds(int textureIndex, sf::Sprite& sprite);
		void applyTextureBounds(int textureIndex, sf::Vertex* quads, int angle = 0); 
		void applyVertexBounds(sf::Vertex* quads, gs::Vec2f position, gs::Vec2f size); 
		void resizeSpriteBounds(sf::Sprite& sprite); 
 
		void updateBackground(); 

		void renderSky(GameTime gameTime);
		void renderBiomeBackground(); 
		void renderStars(GameTime gameTime); 
		void renderSunAndMoon(GameTime gameTime);
		void renderBackground(); 
		void renderChunkLayer(const Chunk& chunk, int layerIndex); 
		void renderWalls(const World& world); 
		void renderBlocks(const World& world); 
		void renderFluids(const World& world); 
		void renderWorldLayer(const World& world, int layerIndex); 
		void renderEntitySegment(
			const Entity& entity, int segmentIndex, int textureOffset = 0, 
			bool flip = false, sf::RenderTarget& target = worldRenderTexture
		); 
		void renderHeldItem(Entity& entity, Item item, ModelTransform::Segments segment);
		void renderPlayerEntity(
			Player& playerEntity, sf::RenderTarget& target = worldRenderTexture
		);
		void renderMobEntity(Mob& mobEntity); 
		void renderItemEntity(const ItemEntity& itemEntity, bool applySin = true); 
		void renderProjectileEntity(const ProjectileEntity& projectileEntity); 
		void renderEntity(const EntityPair& entity); 
		void renderEntities(); 
		void renderParticle(const Particle& particle);
		void renderUnlitParticles(); 
		void renderLitParticles();  
		void renderChunkLightMap(const Chunk& chunk, const World& world); 
		void renderWorldLightMap(const World& world); 
		void finishWorldRendering(); 
	}
}