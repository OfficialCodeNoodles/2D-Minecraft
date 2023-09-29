#include "../../hdr/graphics/Render.hpp"
#include "../../hdr/graphics/UI.hpp"
#include "../../hdr/world/Generation.hpp"
#include "../../hdr/entity/Spawn.hpp"

namespace engine {
	namespace render {
		gs::Vec2f normalizedCameraPosition = gs::Vec2f(0.0f, 0.0f); 
		gs::Vec2f cameraPosition = gs::Vec2f(0.0f, 0.0f);
		gs::Vec2i cameraClipingRange = gs::Vec2i(
			// This calculates the minimum number of tiles required to create a 
			// seemless rendering and rounds the result up to the nearest tile. 
			std::ceil(window::defaultWindowWidth / 2.0f / tileSize) +
				std::fmod(std::ceil(window::defaultWindowWidth / 2.0f / 
					tileSize), tileSize),
			std::ceil(window::defaultWindowHeight / 2.0f / tileSize)
		); 
		float cameraScale = 2.0f;
		gs::Vec2i renderableChunkRange; 
		gs::Vec2i renderableVerticalRange;
		int minimumLightDistance = 15; 
		gs::Vec2i renderableHorizontalLightRange;
		gs::Vec2i renderableVerticalLightRange; 
		gs::Vec2i renderableHorizontalSpawningLightRange;
		gs::Vec2i renderableVerticalSpawningLightRange;

		void updateCamera() {
			normalizedCameraPosition = player->position;

			// Zooming camera in and out. 
			const float zoomStep = 0.25f; 
			const float zoomDivisor = 1.0f / zoomStep; 
			const float zoomSpeed = (std::log2(cameraScale) / 2.0f) + zoomStep;
			const float originalCameraScale = cameraScale; 

			if (!ui::gameOver) {
				if (input::isKeyPressed(input::Zoom))
					cameraScale += zoomSpeed * window::mouseDelta;

				cameraScale = std::round(cameraScale * zoomDivisor) / zoomDivisor;
			}

			gs::util::clamp(&cameraScale, minCameraScale, maxCameraScale); 

			if (cameraScale < originalCameraScale) 
				lighting::forceLights = true;

			// Constrain camera to vertical bounds. 
			const float verticalCameraOffset = window::screenCenter.y
				/ (tileSize * cameraScale);

			gs::util::clamp(
				&normalizedCameraPosition.y, verticalCameraOffset,
				Chunk::height - verticalCameraOffset
			); 

			// Transform camera based on current parameters. 
			cameraPosition = normalizedCameraPosition * tileSize * cameraScale; 
			cameraPosition -= window::screenCenter; 

			cameraPosition.x = std::round(cameraPosition.x); 
			cameraPosition.y = std::round(cameraPosition.y); 

			// Updating what chunks need to be rendered. 
			renderableChunkRange.x = World::getChunkOffset(
				normalizedCameraPosition.x - 
					(cameraClipingRange.x / cameraScale) 
						- (Chunk::width / 2.0f)
			); 
			renderableChunkRange.y = World::getChunkOffset(
				normalizedCameraPosition.x + 
					(cameraClipingRange.x / cameraScale) 
						+ (Chunk::width / 2.0f)
			) + 1;

			// Updating which tiles vertically need to be rendered. 
			renderableVerticalRange.x = normalizedCameraPosition.y 
				- (cameraClipingRange.y / cameraScale); 
			renderableVerticalRange.y = normalizedCameraPosition.y
				+ (cameraClipingRange.y / cameraScale) + 1;

			gs::util::clamp(&renderableVerticalRange.x, 0, Chunk::height);
			gs::util::clamp(&renderableVerticalRange.y, 0, Chunk::height); 

			// Updating which tiles horizontally need to be relit. 
			renderableHorizontalLightRange.x = normalizedCameraPosition.x 
				- (cameraClipingRange.x / cameraScale) - minimumLightDistance;
			renderableHorizontalLightRange.y = normalizedCameraPosition.x 
				+ (cameraClipingRange.x / cameraScale) + minimumLightDistance;

			// Updating which tiles vertically need to be relit. 
			renderableVerticalLightRange.x = normalizedCameraPosition.y
				- (cameraClipingRange.y / cameraScale) - minimumLightDistance;
			renderableVerticalLightRange.y = normalizedCameraPosition.y
				+ (cameraClipingRange.y / cameraScale) + minimumLightDistance;

			gs::util::clamp(&renderableVerticalLightRange.x, 0, Chunk::height); 
			gs::util::clamp(&renderableVerticalLightRange.y, 0, Chunk::height); 

			// Updating which spawning tiles horizontally need to be relit. 
			renderableHorizontalSpawningLightRange.x = -Chunk::width 
				+ normalizedCameraPosition.x - maxSpawningDistance.x
					- lighting::Light::maxLightRadius; 
			renderableHorizontalSpawningLightRange.y = Chunk::width
				+ normalizedCameraPosition.x + maxSpawningDistance.x
					+ lighting::Light::maxLightRadius; 

			// Updating which spawning tiles vertically need to be relit. 
			renderableVerticalSpawningLightRange.x = -Chunk::width
				+ normalizedCameraPosition.y - maxSpawningDistance.y
					- lighting::Light::maxLightRadius; 
			renderableVerticalSpawningLightRange.y = Chunk::width
				+ normalizedCameraPosition.y + maxSpawningDistance.y
					+ lighting::Light::maxLightRadius;

			gs::util::clamp(
				&renderableVerticalSpawningLightRange.x, 0, Chunk::width
			); 
			gs::util::clamp(
				&renderableVerticalSpawningLightRange.y, 0, Chunk::width
			);
		}
		float scaleValue(float value) {
			return value * tileSize * cameraScale; 
		}
		gs::Vec2f scalePosition(gs::Vec2f position) {
			return position * tileSize * cameraScale; 
		}
		gs::Vec2f transformPosition(gs::Vec2f position) {
			gs::Vec2f transformation = position; 

			transformation = scalePosition(transformation); 
			transformation -= cameraPosition; 
			// Round to prevent pixel flipping. 
			transformation.x = std::round(transformation.x);
			transformation.y = std::round(transformation.y);

			return transformation;
		}
		gs::Vec2f transformTilePosition(
			gs::Vec2i chunkPosition, int chunkOffset) 
		{
			const gs::Vec2f transformation = gs::Vec2f(
				chunkPosition.x + (chunkOffset * Chunk::width),
				chunkPosition.y
			); 

			return transformPosition(transformation); 
		}
		const sf::Image& takeScreenshot() {
			static sf::Image baseScreenshot; 

			if (window::winmain->isOpen()) {
				sf::Texture screenTexture;

				screenTexture.create(window::defaultWindowWidth,
					window::defaultWindowHeight);
				screenTexture.update(*window::winmain);

				baseScreenshot = screenTexture.copyToImage();
			}

			return baseScreenshot; 
		}

		gs::Transition transition;
		sf::VertexArray skyBox(sf::Quads, 4); 
		sf::RenderTexture worldRenderTexture;
		sf::Sprite worldRenderTextureSprite;
		bool shouldSunAndMoonBeRendered = true;
		sf::Texture sunTexture, moonTextures;
		sf::Sprite sunSprite, moonSprite;
		bool shouldBiomeBackgroundBeRendered = true;
		std::pair<std::pair<Biome::Id, int>, std::pair<Biome::Id, int>> 
			biomeBackgrounds;
		Biome::Id prvsBiome;
		sf::Texture biomeBackgroundTextures[Biome::End];
		sf::Sprite biomeBackgroundSprite;
		bool shouldParticlesBeRendered = true; 
		bool shouldStarsBeRendered = true; 
		float starBrightness = 0.0f; 
		sf::Texture blockAtlas, wallAtlas;
		sf::Image blockAtlasImage, wallAtlasImage;
		sf::Texture blockOverlayAtlas;
		sf::Sprite blockSprite, wallSprite; 
		sf::Vertex tileVerticies[maxNumOfTiles * 4];
		sf::Vertex tileOverlayVerticies[maxNumOfTiles * 4];
		int tileVertexIndex = 0, tileOverlayVertexIndex = 0; 
		sf::RenderTexture lightMap;
		sf::Sprite lightMapSprite;
		sf::Vertex lightMapVerticies[maxNumOfTiles * 4];
		int lightVertexIndex = 0; 
		sf::Texture tileBreakingStageTextures[10];
		sf::Sprite tileBreakingStageSprite;
		int tilesRendered = 0;
		sf::Texture entitySegmentTextures[Model::End]
			[Model::Segment::maxNumOfSegments];
		sf::Sprite entitySegmentSprite;

		void loadAssets() {
			// Load sun & moon textures. 
			addResourceLoader(ResourceLoader(
				ResourceLoader::Type::Texture, assetDirectory
					+ "textures/enviroment/sun.png", &sunTexture, &sunSprite
			)); 
			addResourceLoader(ResourceLoader(
				ResourceLoader::Type::Texture, assetDirectory
					+ "textures/enviroment/moons.png", &moonTextures, 
				&moonSprite
			));

			// Load biome background textures. 
			for (int biomeIndex = 0; biomeIndex < Biome::End; biomeIndex++) {
				const std::string& biomeString = 
					Biome::biomeStrings[biomeIndex]; 
				
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/enviroment/biomeBackgrounds/"
						+ biomeString + ".png", 
					&biomeBackgroundTextures[biomeIndex]
				)); 
			}

			// Load atlases.
			addResourceLoader(ResourceLoader(
				ResourceLoader::Type::Texture, assetDirectory
					+ "textures/atlases/block.png", &blockAtlas, &blockSprite
			)); 
			addResourceLoader(ResourceLoader(
				ResourceLoader::Type::Texture, assetDirectory
					+ "textures/atlases/wall.png", &wallAtlas, &wallSprite
			));
			addResourceLoader(ResourceLoader(
				ResourceLoader::Type::Texture, assetDirectory
					+ "textures/atlases/blockOverlay.png", &blockOverlayAtlas
			));

			// Load tile breaking stages. 
			for (int tileBreakingStage = 0; tileBreakingStage < 10; 
				tileBreakingStage++) 
			{ 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/misc/tileBreakingStages/"
							"tileBreakingStage" + toString(tileBreakingStage)
						+ ".png",
					&tileBreakingStageTextures[tileBreakingStage]
				)); 
			}

			// Load entity textures. 
			for (int modelIndex = 1; modelIndex < Model::End; modelIndex++) {
				const std::string& modelName = Model::modelStrings[modelIndex]; 

				sf::Texture* modelTextures = entitySegmentTextures[modelIndex]; 
				int textureIndex = 0; 

				for (int segmentIndex = 0; segmentIndex <
					Model::Segment::maxNumOfSegments; segmentIndex++)
				{
					const std::string filename = assetDirectory + "textures/entity/"
						+ modelName + "/" + modelName + toString(textureIndex)
						+ ".png";

					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, filename,
						&modelTextures[segmentIndex]
					)); 
					textureIndex++;
				}
			}
		}
		void handleAssets() {
			// Initalize transition. 
			transition.color = gs::Color::White; 
			transition.inc = 10.0f; 

			// Initialize skyBox.  
			skyBox[0].position = gs::Vec2f(0.0f, 0.0f); 
			skyBox[1].position = gs::Vec2f(window::defaultWindowWidth, 0.0f); 
			skyBox[2].position = gs::Vec2f(
				window::defaultWindowWidth, window::defaultWindowHeight
			); 
			skyBox[3].position = gs::Vec2f(0.0f, window::defaultWindowHeight); 

			// Initialize worldRenderTexture. 
			worldRenderTexture.create(window::defaultWindowWidth, window::defaultWindowHeight); 
			worldRenderTextureSprite.setTexture(worldRenderTexture.getTexture()); 

			// Initialize sun & moon sprites.  

			const int sunAndMoonWidth = 32; 

			sunSprite.setOrigin(sunAndMoonWidth / 2.0f, sunAndMoonWidth / 2.0f); 
			moonSprite.setOrigin(sunSprite.getOrigin()); 

			// Initialize biomebackgrounds. 
			biomeBackgrounds.second.first = Biome::Id::Forest; 

			// Initialize block & wall atlas images. 
			blockAtlasImage = blockAtlas.copyToImage(); 
			wallAtlasImage = wallAtlas.copyToImage(); 

			// Initialize lightMap. 
			lightMap.create(window::defaultWindowWidth, window::defaultWindowHeight); 
			lightMapSprite.setTexture(lightMap.getTexture());
		}
		float getBaseSkyBrightness(GameTime::GameTick gameTicks) {
			const float c0 = 320.0f, c1 = 230.0f;
			const float floatTicks = static_cast<float>(gameTicks);
			const float halfMaxTime = GameTime::maxTime / 2.0f;
			const float brightness = std::pow((floatTicks - halfMaxTime)
				/ halfMaxTime, 2.0f) * (100.0f + c0);

			return gs::util::clamp(brightness - c0 + c1, 0.0f, 100.0f);
		}
		int getBlockTextureIndex(Block block) {
			int textureIndex = BlockInfo::getVar(block.id,
				BlockInfo::textureIndex);
			textureIndex += block.tags.animationOffset; 
			return textureIndex;
		}
		int getWallTextureIndex(Wall wall) {
			int textureIndex = wall.id;
			return textureIndex;
		}
		gs::Color getTilePixelColor(int textureIndex, const sf::Image& tileAtlasImage) {
			const sf::IntRect textureBounds = generateTextureBounds(textureIndex); 
			const int textureArea = textureBounds.width * textureBounds.height; 

			gs::Color generatedColor; 
			int pixelIndex; 
			int generationAttempts = 0; 

			do {
				// Selects a random pixel within the texture. 
				pixelIndex = randomGenerator.generate() % textureArea; 
				// Gets the color of that selected pixel. 
				generatedColor = tileAtlasImage.getPixel(
					textureBounds.left + (pixelIndex % textureBounds.width),
					textureBounds.top + std::floor(pixelIndex / textureBounds.height)
				);
				generationAttempts++; 
			} 
			while (gs::Color(generatedColor.r, generatedColor.g, generatedColor.b) 
					== gs::Color::Black && generationAttempts < 100);

			return generatedColor; 
		}
		gs::Color getBlockPixelColor(Block::Id blockId) {
			return getTilePixelColor(blockId, blockAtlasImage); 
		}
		gs::Color getWallPixelColor(Wall::Id wallId) {
			return getTilePixelColor(wallId, wallAtlasImage); 
		}
		sf::IntRect generateTextureBounds(int textureIndex) {
			const gs::Vec2i textureSize = gs::Vec2i(tileSize, tileSize); 
			const int sheetWidth = 10; 

			sf::IntRect bounds; 

			bounds.width = textureSize.x; 
			bounds.height = textureSize.y; 

			bounds.left = ((textureIndex % sheetWidth) * tileSize)
				// Accounts for the 1 pixel gap between tiles. 
				+ (textureIndex % sheetWidth);
			bounds.top = (std::floor(textureIndex / sheetWidth) * tileSize)
				// Same thing here aswell.
				+ std::floor(textureIndex / sheetWidth);

			return bounds; 
		}
		void applyTextureBounds(int textureIndex, sf::Sprite& sprite) {
			sprite.setTextureRect(generateTextureBounds(textureIndex)); 
		}
		void applyTextureBounds(int textureIndex, sf::Vertex* quads, int angle) {
			auto generateTextureCoord = [](int index, sf::IntRect bounds) 
				-> gs::Vec2f 
			{
				gs::Vec2f textureCoord = gs::Vec2f(bounds.left, bounds.top); 

				textureCoord.x += index > 0 && index < 3 ? bounds.width : 0; 
				textureCoord.y += index > 1 ? bounds.height : 0; 

				return textureCoord; 
			}; 

			const sf::IntRect bounds = generateTextureBounds(textureIndex);

			for (int quad = 0; quad < 4; quad++) {
				quads[quad].texCoords = generateTextureCoord(
					(quad + angle) % 4, bounds
				); 
			}
		}
		void applyVertexBounds(
			sf::Vertex* quads, gs::Vec2f position, gs::Vec2f size)
		{
			quads[0].position = position; 
			quads[1].position = position + gs::Vec2f(size.x, 0.0f);
			quads[2].position = position + size; 
			quads[3].position = position + gs::Vec2f(0.0f, size.y);
		}
		void resizeSpriteBounds(sf::Sprite& sprite) {
			const gs::Vec2u textureSize = sprite.getTexture()->getSize();

			sprite.setTextureRect(sf::IntRect(
				gs::Vec2i(), gs::Vec2i(textureSize.x, textureSize.y)
			));
		}

		void updateBackground() {
			if (!renderBiomeBackground)
				return; 

			const int currentChunkOffset = World::getChunkOffset(player->position.x);
			const Biome::Id currentBiome = getBiome(currentChunkOffset);

			decltype(biomeBackgrounds.first)* biomeBackground = 
				&biomeBackgrounds.first; 
			decltype(biomeBackgrounds.first)* secondaryBiomeBackground =
				&biomeBackgrounds.second; 

			if (currentBiome != prvsBiome) {
				const bool initializeBackground = currentBiome 
					!= biomeBackground->first && currentBiome 
					!= secondaryBiomeBackground->first; 
					
				if (biomeBackgrounds.first.first != prvsBiome);
				else if (biomeBackgrounds.second.first != prvsBiome)
					std::swap(biomeBackgrounds.first, biomeBackgrounds.second);

				// Creates new background for rendering if biome isn't one of
				// the pre-existing backgrounds. 
				if (initializeBackground) {
					biomeBackground->first = currentBiome;
					biomeBackground->second = 0.0f;
				}
			}

			// Makes back background brighter. 
			biomeBackground->second = std::min(biomeBackground->second + 5, 255); 
			// Makes front background darker until fully transparent. 
			secondaryBiomeBackground->second = std::max(
				secondaryBiomeBackground->second - 5, 0
			); 
			
			prvsBiome = currentBiome; 
		}

		void renderSky(GameTime gameTime) {
			auto skyBrightnessParabola = [](GameTime::GameTick gameTicks) 
				-> float
			{
				const float divisor = 1.1f; 
				const float baseBrightness = getBaseSkyBrightness(gameTicks);
				return (baseBrightness / divisor);
			};

			const int numOfSkyColors = 6; 
			const gs::Color topSkyColors[numOfSkyColors] = {
				gs::Color(0, 0, 0), gs::Color(110, 180, 255),
				gs::Color(90, 160, 255), gs::Color(70, 150, 255),
				gs::Color(80, 170, 255), gs::Color(120, 15, 255)
			};
			const gs::Color bottomSkyColors[numOfSkyColors] = {
				gs::Color(0, 0, 0), gs::Color(255, 255, 100),
				gs::Color(100, 180, 255), gs::Color(80, 160, 255),
				gs::Color(100, 160, 255), gs::Color(255, 190, 120)
			};

			gameTime.tick(-2000 + GameTime::maxTime); 

			const int colorTime = GameTime::maxTime / numOfSkyColors; 
			const int colorTimeDifference = gameTime.gameTicks % colorTime; 
			const float colorBlendPercentage = (static_cast<float>(
				colorTimeDifference) / static_cast<float>(colorTime)) * 100.0f; 
			const int colorIndex0 = gameTime.gameTicks / colorTime;
			const int colorIndex1 = (colorIndex0 + 1) % numOfSkyColors; 

			const gs::Color baseTopSkyColor = gs::util::approach(
				topSkyColors[colorIndex0], topSkyColors[colorIndex1],
				colorBlendPercentage
			);
			const gs::Color baseBottomSkyColor = gs::util::approach(
				bottomSkyColors[colorIndex0], bottomSkyColors[colorIndex1],
				colorBlendPercentage
			);
			const float baseSkyBrightness = 
				getBaseSkyBrightness(gameTime.gameTicks);
			
			gs::Color baseSkyColor = gs::util::approach(
				baseTopSkyColor, baseBottomSkyColor, 80.0f);
			gs::util::approach(&baseSkyColor, gs::Color::White, 
				100.0f - baseSkyBrightness);  

			const float skyBrightness = skyBrightnessParabola(
				gameTime.gameTicks); 
			const gs::Color currentTopSkyColor = gs::util::approach(
				baseTopSkyColor, lighting::ambientLightColor, skyBrightness);
			const gs::Color currentBottomSkyColor = gs::util::approach(
				baseBottomSkyColor, lighting::ambientLightColor, skyBrightness);

			lighting::sunlightColor = baseSkyColor;
			lighting::sunlightBrightness = baseSkyBrightness;

			skyBox[0].color = skyBox[1].color = currentTopSkyColor; 
			skyBox[2].color = skyBox[3].color = currentBottomSkyColor; 

			window::winmain->draw(skyBox);
		}
		void renderStars(GameTime gameTime) {
			if (!shouldStarsBeRendered)
				return; 

			GameTime offsetGameTime = gameTime; 

			offsetGameTime.tick(22000); 

			starBrightness = getBaseSkyBrightness(offsetGameTime.gameTicks);

			if (starBrightness == 0.0f)
				return; 

			for (auto& particle : particles) {
				if (particle.type == Particle::Type::Star) 
					renderParticle(particle); 
			}
		}
		void renderSunAndMoon(GameTime gameTime) {
			auto getSunAndMoonAngle = [](GameTime gameTime, bool isSun)
				-> float
			{
				gameTime.tick(-2000); 
				
				float angle = static_cast<float>(gameTime.gameTicks)
					/ static_cast<float>(gameTime.maxTime) * 360.0f;

				if (!isSun) angle += 180.0f; 

				return angle; 
			};
			auto getSunAndMoonPosition = [&](GameTime gameTime, bool isSun) 
				-> gs::Vec2f 
			{
				const float verticalScaler = 800.0f; 
				const float horizontalScaler = 700.0f; 
				const gs::Vec2f offset = gs::Vec2f(
					window::defaultWindowWidth / 2.0f,
					window::defaultWindowHeight / 1.2f
				); 
				const float angle = getSunAndMoonAngle(gameTime, isSun); 

				return offset + gs::Vec2f(
					horizontalScaler * std::sin(-gs::util::toRadians(angle)),
					verticalScaler * std::cos(-gs::util::toRadians(angle))
				); 
			}; 
			auto getSunAndMoonAlpha = [&](GameTime gameTime, bool isSun) -> int {
				const float angle = std::fmod(
					getSunAndMoonAngle(gameTime, isSun) + 300.0f,
					360.0f
				);

				float normalizedAlpha = (std::sin(gs::util::toRadians(angle)) 
					/ 2.0f) + 0.5f;

				if (isSun) 
					normalizedAlpha = std::min(normalizedAlpha + 0.3f, 1.0f); 

				return std::pow(normalizedAlpha, 1.2f) * 255.0f;
			};

			const int moonWidth = 32; 
			const int moonPhase = 0 % 8; 

			sf::IntRect bounds; 

			bounds.width = moonWidth; 
			bounds.height = moonWidth; 

			bounds.left = moonPhase * moonWidth;

			moonSprite.setTextureRect(bounds); 

			sunSprite.setScale(16.0f, 16.0f);
			sunSprite.setPosition(getSunAndMoonPosition(gameTime, true)); 
			sunSprite.setColor(gs::Color(
				255, 255, 255, getSunAndMoonAlpha(gameTime, true)
			));

			window::winmain->draw(sunSprite); 

			moonSprite.setScale(sunSprite.getScale()); 
			moonSprite.setPosition(getSunAndMoonPosition(gameTime, false));
			moonSprite.setColor(gs::Color(
				255, 255, 255, getSunAndMoonAlpha(gameTime, false)
			));

			window::winmain->draw(moonSprite);
		}
		void renderBiomeBackground() {
			const decltype(biomeBackgrounds.first)* sortedBiomeBackgrounds[2] = {
				&biomeBackgrounds.second, &biomeBackgrounds.first
			}; 

			for (auto* biomeBackground : sortedBiomeBackgrounds) {
				// Don't render biome background if fully transparent. 
				if (biomeBackground->second == 0)
					continue;

				// Texture selected based on the biome. 
				sf::Texture& selectedTexture = biomeBackgroundTextures
					[biomeBackground->first]; 

				biomeBackgroundSprite.setTexture(selectedTexture); 
				resizeSpriteBounds(biomeBackgroundSprite); 

				const float biomeBackgroundScale = 1.0f + std::log2(cameraScale);
				const float biomeBackgroundWidth = selectedTexture.getSize().x;
				const float horizontalOffset = gs::util::mod(
					-player->position.x * 5.0f * biomeBackgroundScale,
					biomeBackgroundWidth * biomeBackgroundScale
				);

				biomeBackgroundSprite.setScale(biomeBackgroundScale, biomeBackgroundScale);
				biomeBackgroundSprite.setOrigin(
					selectedTexture.getSize().x / 2.0f,
					selectedTexture.getSize().y / 2.0f
				);

				const int backgroundEndPoint = horizontalOffset 
					> (biomeBackgroundWidth + (((biomeBackgroundWidth 
						* biomeBackgroundScale) - biomeBackgroundWidth) * 0.5f));

				// Only render the number of backgrounds required to fill the 
				// screen.
				for (int renderCount = 2; renderCount > backgroundEndPoint; renderCount--) {
					const float backgroundShadow = 12.0f; 
					const float backgroundVerticalOffset = 80.0f; 
					
					biomeBackgroundSprite.setPosition(biomeBackgroundSprite.getOrigin());
					biomeBackgroundSprite.move(horizontalOffset, backgroundVerticalOffset);
					biomeBackgroundSprite.move(-biomeBackgroundWidth
						* (renderCount - 1) * biomeBackgroundScale, 0.0f);
					
					gs::Color biomeBackgroundColor = gs::util::approach(
						lighting::sunlightColor, lighting::moonlightColor,
						lighting::sunlightBrightness
					);
					biomeBackgroundColor -= gs::Color(
						backgroundShadow, backgroundShadow, backgroundShadow
					); 
					biomeBackgroundColor.a = biomeBackground->second; 

					biomeBackgroundSprite.setColor(biomeBackgroundColor);

					window::winmain->draw(biomeBackgroundSprite);
				}
			}
		}
		void renderBackground() {
			renderSky(world->gameTime); 
			renderStars(world->gameTime); 

			if (shouldSunAndMoonBeRendered)
				renderSunAndMoon(world->gameTime); 
			if (shouldBiomeBackgroundBeRendered)
				renderBiomeBackground(); 
		}
		void renderChunkLayer(const Chunk& chunk, int layerIndex) {
			auto calculateHorizontalOffset = [](gs::Vec2i position) -> int {
				return -4 + (std::abs(position.x * 123) % 9); 
			}; 

			const bool isBlockLayer = layerIndex > 0;
			const gs::Color biomeColors[3] = {
				BiomeInfo::biomeInfo[getBiome(chunk.offset - 1)].color,
				BiomeInfo::biomeInfo[getBiome(chunk.offset)].color,
				BiomeInfo::biomeInfo[getBiome(chunk.offset + 1)].color
			};

			gs::Vec2i tilePosition;

			for (tilePosition.x = 0; tilePosition.x < Chunk::width; 
				tilePosition.x++) 
			{
				// Color used for plant overlays. 
				gs::Color overlayColor; 

				const float biomeColorPercentage = ((static_cast<float>(tilePosition.x)
					/ static_cast<float>(Chunk::width)) * 100.0f) + 50.0f;
				const int biomeColorIndex = biomeColorPercentage / 100; 

				// Calculates overlay color by interpolating between biomes. 
				overlayColor = gs::util::approach(
					biomeColors[biomeColorIndex], 
					biomeColors[biomeColorIndex + 1], 
					std::fmod(biomeColorPercentage, 100.0f)
				);

				for (tilePosition.y = renderableVerticalRange.x; 
					tilePosition.y < renderableVerticalRange.y; 
					tilePosition.y++) 
				{
					const Block block = chunk.getBlock(tilePosition);

					gs::Vec2f renderPosition = transformTilePosition(
						tilePosition, chunk.offset
					);
					gs::Vec2f renderSize = scalePosition({ 1.0f, 1.0f }); 
					int textureIndex = 0; 
					int rotation = 0;

					if (layerIndex > 0) {
						const bool foreground = BlockInfo::getVar(
							block.id, BlockInfo::foreground); 

						if (layerIndex != (1 + static_cast<int>(foreground)))
							continue; 
						
						if (block.getVar(BlockInfo::horizontalShift))
							renderPosition.x += cameraScale 
								* calculateHorizontalOffset(tilePosition); 
						// Allows the fluids to change height, depending on
						// the level. 
						if (block.isFluid()) {
							float offset = cameraScale 
								* block.tags.fluidLevel * 2.0f; 

							if (block.tags.fluidLevel == 0 
								&& chunk.getBlockId(tilePosition 
									- gs::Vec2i(0, 1)) != block.id)
								offset = cameraScale; 

							renderPosition.y += offset;
							renderSize.y -= offset; 
						}
						else if (static_cast<BlockInfo::BlockDependencyType>(
							block.getVar(BlockInfo::blockDependencyType))
								== BlockInfo::BlockDependencyType::Crop)
							renderPosition.y += cameraScale; 

						textureIndex = getBlockTextureIndex(block); 
						rotation = block.tags.rotation; 
					}
					else {
						const Wall wall = chunk.getWall(tilePosition); 

						if (!BlockInfo::getVar(block.id,
								BlockInfo::hasTransparency))
							continue; 

						textureIndex = getWallTextureIndex(wall); 
					}

					// Makes sure the tiles texture exists. 
					if (textureIndex != 0) {
						bool renderBlockOverlay = false; 

						if (isBlockLayer) {
							renderBlockOverlay = BlockInfo::getVar(block.id, 
								BlockInfo::hasBlockOverlay);

							if (renderBlockOverlay) {
								// Ignore normal block rendering if not required. 
								if (!BlockInfo::getVar(block.id,
										BlockInfo::renderUnderside))
									goto RENDER_OVERLAY; 
							}
						}

						applyVertexBounds(
							&tileVerticies[tileVertexIndex], renderPosition, 
							renderSize
						); 
						applyTextureBounds(
							textureIndex, &tileVerticies[tileVertexIndex], 
							rotation
						); 

						tilesRendered++; 
						tileVertexIndex += 4; 

						if (renderBlockOverlay) [[unlikely]] {
RENDER_OVERLAY:
							for (int quad = 0; quad < 4; quad++) {
								tileOverlayVerticies[tileOverlayVertexIndex
									+ quad].color = overlayColor;
							}

							applyVertexBounds(
								&tileOverlayVerticies[tileOverlayVertexIndex], 
								renderPosition, renderSize
							);
							applyTextureBounds(
								textureIndex, 
								&tileOverlayVerticies[tileOverlayVertexIndex],
								rotation
							);

							tileOverlayVertexIndex += 4;
						}
					}
				}
			}
		}
		void renderWalls(const World& world) {
			renderWorldLayer(world, 0); 

			if (!blockInteraction)
				ui::renderTileBreaking();
		}
		void renderBlocks(const World& world) {
			renderWorldLayer(world, 1); 

			if (blockInteraction) 
				ui::renderTileBreaking();
		}
		void renderFluids(const World& world) {
			renderWorldLayer(world, 2); 
		}
		void renderWorldLayer(const World& world, int layerIndex) {
			const bool isBlockLayer = layerIndex > 0;

			tileVertexIndex = 0; 
			tileOverlayVertexIndex = 0; 

			if (layerIndex == 0) {
				worldRenderTexture.clear(gs::Color::Transparent); 
				tilesRendered = 0;
			}

			for (int chunkOffset = renderableChunkRange.x; chunkOffset < 
				renderableChunkRange.y; chunkOffset++) 
			{
				const Chunk* chunk = world.getChunk(chunkOffset);   

				if (chunk != nullptr)
					renderChunkLayer(*chunk, layerIndex); 
			}

			worldRenderTexture.draw(
				tileVerticies, tileVertexIndex, sf::Quads,
				&(layerIndex == 0 ? wallAtlas : blockAtlas)
			);

			if (isBlockLayer) {
				// Render block overlay. 
				worldRenderTexture.draw(
					tileOverlayVerticies, tileOverlayVertexIndex, sf::Quads,
					&blockOverlayAtlas
				);
			}
		}
		void renderEntitySegment(
			const Entity& entity, int segmentIndex, int textureOffset, 
			bool flip, sf::RenderTarget& target)
		{
			const Model::Id modelId = entity.modelTransform.id; 
			// The static segment object being rendered. 
			const Model::Segment& segment =
				Model::models[modelId].segments[segmentIndex];
			// The information that transforms and animates the segment. 
			const ModelTransform::SegmentTransform& segmentTransform = 
				entity.modelTransform.transforms[segmentIndex]; 

			entitySegmentSprite.setScale(cameraScale, cameraScale); 
			entitySegmentSprite.setTexture(
				entitySegmentTextures[modelId]
					[segment.textureIndex + textureOffset]
			);
			resizeSpriteBounds(entitySegmentSprite); 

			entitySegmentSprite.setOrigin(gs::Vec2f(
				flip ? entitySegmentSprite.getTexture()->getSize().x 
					- segment.origin.x : segment.origin.x,
				segment.origin.y
			)); 
			entitySegmentSprite.setPosition(transformPosition(
				entity.position + gs::Vec2f(segment.position.x 
					* (flip ? -1.0f : 1.0f), segment.position.y)
			)); 
			entitySegmentSprite.setRotation(
				segment.rotation + segmentTransform.angle
			); 
			entitySegmentSprite.setColor(
				segment.color * segmentTransform.color
			);

			target.draw(entitySegmentSprite);

			entitySegmentSprite.setColor(gs::Color::White); 
		}
		void renderHeldItem(
			Entity& entity, Item item, ModelTransform::Segments segment) 
		{
			const float distanceAlongAppendage = 10.0f / tileSize;
			const float scaler = entity.modelTransform.facingForward
				? 1.0f : -1.0f;
			const Model::Segment& appendage = Model::models[
				entity.modelTransform.id].segments[segment];
			const float itemAngle = entity.modelTransform.
				transforms[segment].angle + 90.0f;
			const float itemAngleOffset = item.getVar(ItemInfo::angleOffset)
				* -scaler; 

			ItemEntity heldItem;

			heldItem.position = entity.position + appendage.position;
			heldItem.position += gs::Vec2f(
				distanceAlongAppendage * std::cos(
					gs::util::toRadians(itemAngle + itemAngleOffset)),
				distanceAlongAppendage * std::sin(
					gs::util::toRadians(itemAngle + itemAngleOffset))
			);
			heldItem.position += gs::Vec2f(
				player->size.x * std::cos(gs::util::toRadians(
					itemAngle - 90.0f)),
				player->size.x * std::sin(gs::util::toRadians(
					itemAngle - 90.0f))
			) * scaler;
			heldItem.itemContainer.item = item; 

			float animationAngle =
				ItemInfo::itemInfo[heldItem.itemContainer.item.id]
					.getVar(ItemInfo::animationAngle);
			// True if the animation angle is negative. 
			const bool flipItem = animationAngle < 0.0f;
			animationAngle = std::abs(animationAngle);

			ui::itemSprite.setScale(
				ui::itemSprite.getScale().x * -1.0f,
				ui::itemSprite.getScale().y * scaler
				* (flipItem ? -1.0f : 1.0f)
			);
			ui::itemSprite.setRotation(
				itemAngle + animationAngle * scaler
			);

			renderItemEntity(heldItem, false);

			ui::itemSprite.setScale(
				std::abs(ui::itemSprite.getScale().x),
				std::abs(ui::itemSprite.getScale().y)
			);
			ui::itemSprite.setRotation(0.0f);
		}
		void renderPlayerEntity(Player& playerEntity, sf::RenderTarget& target) {
			const float scaler = playerEntity.modelTransform.facingForward 
				? 1.0f : -1.0f; 

			float& frontArmAngleRef = playerEntity.modelTransform.
				transforms[ModelTransform::FrontArm].angle;
			float& backArmAngleRef = playerEntity.modelTransform.
				transforms[ModelTransform::BackArm].angle; 

			// Handle mining animation. 
			if (playerEntity.miningDuration != 0.0f) {
				frontArmAngleRef = (std::sin(playerEntity.miningDuration / 3.0f) 
					* 20.0f) + 40.0f;
				frontArmAngleRef *= -scaler;
			}

			const ItemContainer& itemContainer = 
				playerEntity.inventory[ui::hotbarIndex];
			                                                          
			for (int segmentIndex = 0; segmentIndex < 
				Model::modelLimbCount[playerEntity.modelTransform.id]; 
				segmentIndex++) 
			{
				if (segmentIndex == ModelTransform::FrontArm 
						&& itemContainer.count > 0) 
					renderHeldItem(playerEntity, itemContainer.item, 
						ModelTransform::FrontArm); 

				renderEntitySegment(
					playerEntity, segmentIndex, 
					playerEntity.modelTransform.facingForward ? 0 : 4, 
					!playerEntity.modelTransform.facingForward
				);
			}
		}
		void renderMobEntity(Mob& mobEntity) {
			for (int segmentIndex = 0; segmentIndex < 
				Model::modelLimbCount[mobEntity.modelTransform.id];
				segmentIndex++) 
			{
				switch (mobEntity.type) {
				case Mob::Skeleton:
					if (segmentIndex == ModelTransform::FrontArm) {
						renderHeldItem(mobEntity, Item(Item::Bow), 
							ModelTransform::FrontArm); 
					}

					break; 
				}

				renderEntitySegment(
					mobEntity, segmentIndex,
					mobEntity.modelTransform.facingForward ? 0 : 4, 
					!mobEntity.modelTransform.facingForward
				);
			}
		}
		void renderItemEntity(const ItemEntity& itemEntity, bool applySin) {
			// Item's vertical offset center. 
			const float distanceOffGround = (1.0f - itemEntity.size.y) / 4.0f;
			// Calculates how high off of the ground the item should hover. 
			// Note: This offset follows a sin wave. 
			const float verticalOffset = distanceOffGround * std::sin(
				(static_cast<float>(window::ticks) / 10.0f)
					+ (itemEntity.position.x * 2.5f)
			);
			const gs::Vec2f renderPosition = transformPosition(gs::Vec2f(
				itemEntity.position.x, itemEntity.position.y 
					+ (applySin ? -distanceOffGround + verticalOffset : 0.0f)
			));

			ui::renderItem(
				itemEntity.itemContainer.item, renderPosition,
				scaleValue(itemEntity.size.x), worldRenderTexture
			); 
		}
		void renderProjectileEntity(const ProjectileEntity& projectileEntity) {
			entitySegmentSprite.setTexture(
				entitySegmentTextures[Model::Arrow][0]
			); 
			resizeSpriteBounds(entitySegmentSprite); 

			entitySegmentSprite.setOrigin(
				gs::Vec2f(entitySegmentSprite.getTexture()->getSize()) * 0.5f
			); 
			entitySegmentSprite.setPosition(
				transformPosition(projectileEntity.position)
			); 
			entitySegmentSprite.setRotation(projectileEntity.angle); 
			entitySegmentSprite.setColor(gs::Color(
				255, 255, 255, std::min(projectileEntity.timeLeft * 5, 255)
			)); 

			worldRenderTexture.draw(entitySegmentSprite); 

			entitySegmentSprite.setRotation(0.0f); 
			entitySegmentSprite.setColor(gs::Color::White); 
		}
		void renderEntity(const EntityPair& entity) {
			// Call render function corresponding to the entity type. 
			switch (entity.first) {
			case Entity::Type::PlayerEntity:
				if (!ui::gameOver)
					renderPlayerEntity(dynamic_cast<Player&>(*entity.second)); 
				break; 
			case Entity::Type::Mob:
				renderMobEntity(dynamic_cast<Mob&>(*entity.second)); 
				break; 
			case Entity::Type::ItemEntity:
				renderItemEntity(
					dynamic_cast<const ItemEntity&>(*entity.second)
				); 
				break; 
			case Entity::Type::ProjectileEntity:
				renderProjectileEntity(
					dynamic_cast<const ProjectileEntity&>(*entity.second)
				); 
				break; 
			}
		}
		void renderEntities() {
			for (auto& entity : entities) {
				renderEntity(entity); 
			}
		}
		void renderParticle(const Particle& particle) {
			static sf::RectangleShape particleBox; 
			static sf::Text particleText; 

			switch (particle.type) {
			case Particle::Type::Generic:
				particleBox.setSize(scalePosition(particle.size)); 
				particleBox.setOrigin(particleBox.getSize() * 0.5f);
				particleBox.setPosition(transformPosition(particle.position));
				particleBox.setRotation(particle.rotation); 
				particleBox.setFillColor(particle.color); 

				worldRenderTexture.draw(particleBox);
				break; 
			case Particle::Type::Star: 
			{
				const float transparencyScaler = 1.0f - (particle.position.y 
					/ Particle::starLowerHorizon);
				float transparency = particle.color.a * transparencyScaler; 

				gs::util::approach(&transparency, 0.0f, 100.0f - starBrightness); 

				particleBox.setSize(particle.size);
				particleBox.setOrigin(particleBox.getSize() * 0.5f);
				particleBox.setPosition(particle.position);
				particleBox.setRotation(particle.rotation);
				particleBox.setFillColor(gs::Color(
					particle.color.r, particle.color.g, particle.color.b,
					transparency
				)); 
					
				window::winmain->draw(particleBox);
			}
				break; 
			case Particle::Type::DamagePoint:
			{
				const float transparency = std::min(
					particle.timeRemaining * 10.0f, 255.0f
				); 
				const gs::Vec2f position = gs::Vec2f(
					particle.position.x, particle.position.y + std::sin(
						static_cast<float>(particle.timeRemaining) / 10.0f) * 0.1f
				); 

				particleText.setFont(ui::fonts[0]); 
				particleText.setString(particle.string); 
				particleText.setPosition(transformPosition(position)); 
				particleText.setScale(gs::Vec2f(
					// Scales text with the camera's scale. 
					cameraScale / 6.0f, cameraScale / 6.0f)
				); 
				particleText.setFillColor(gs::Color(
					particle.color.r, particle.color.g, particle.color.b, 
					transparency
				)); 

				window::winmain->draw(particleText); 
			}
				break; 
			}
			
			particlesRendered++; 
		}
		void renderUnlitParticles() {
			if (!shouldParticlesBeRendered)
				return;

			for (auto& particle : particles) {
				const bool litParticle =
					Particle::litParticles[static_cast<int>(particle.type)];

				if (particle.active && !litParticle 
						&& particle.type != Particle::Type::Star)
					renderParticle(particle);
			}
		}
		void renderLitParticles() {
			if (!shouldParticlesBeRendered)
				return; 

			for (auto& particle : particles) {
				const bool litParticle = 
					Particle::litParticles[static_cast<int>(particle.type)]; 

				if (particle.active && litParticle) 
					renderParticle(particle);
			}
		}
		void renderChunkLightMap(const Chunk& chunk, const World& world) {
			auto blendQuad = [&](gs::Vec2i position) -> TileColor {
				// Stores the sum of each component. 
				int colorValues[3] = { 0, 0, 0 };
				gs::Vec2i tilePosition;

				for (tilePosition.x = position.x; tilePosition.x < 
					position.x + 2; tilePosition.x++) 
				{
					for (tilePosition.y = position.y;
						tilePosition.y < position.y + 2; tilePosition.y++)
					{
						const TileColor tileColor = 
							world.getTileColor(tilePosition);

						// Adds components to total. 
						colorValues[0] += tileColor.r; 
						colorValues[1] += tileColor.g; 
						colorValues[2] += tileColor.b; 
					}
				}

				// Finds average at the end by dividing the total of each 
				// component by 4. 
				return TileColor(
					colorValues[0] / 4, 
					colorValues[1] / 4,
					colorValues[2] / 4
				); 
			};

			gs::Vec2i tilePosition;

			for (tilePosition.x = 0; tilePosition.x < Chunk::width;
				tilePosition.x++)
			{
				for (tilePosition.y = renderableVerticalRange.x;
					tilePosition.y < renderableVerticalRange.y;
					tilePosition.y++)
				{
					const gs::Vec2f renderPosition = transformTilePosition(
						tilePosition, chunk.offset);
					const TileColor tileColor = chunk.getTileColor(tilePosition);

					switch (lighting::lightingStyle) {
					case lighting::LightingStyle::Geometry:
						if (tileColor == lighting::ambientLightColor)
							continue;  

						for (int quad = 0; quad < 4; quad++)
							lightMapVerticies[lightVertexIndex + quad].color = tileColor; 

						break;
					case lighting::LightingStyle::Smooth:
					{
						const gs::Vec2i positionInWorld = gs::Vec2i(
							tilePosition.x + (chunk.offset * Chunk::width),
							tilePosition.y
						);

						lightMapVerticies[lightVertexIndex + 0].color = blendQuad(
							positionInWorld + gs::Vec2i(-1, -1));
						lightMapVerticies[lightVertexIndex + 1].color = blendQuad(
							positionInWorld + gs::Vec2i(0, -1));
						lightMapVerticies[lightVertexIndex + 2].color = blendQuad(
							positionInWorld + gs::Vec2i(0, 0));
						lightMapVerticies[lightVertexIndex + 3].color = blendQuad(
							positionInWorld + gs::Vec2i(-1, 0)); 

						// Stops rendering if completely dark. 
						for (int offset = 0; offset < 4; offset++) {
							if (lightMapVerticies[lightVertexIndex + offset].color
									!= lighting::ambientLightColor)
								continue;
						}
					}
						break;
					}

					applyVertexBounds(
						&lightMapVerticies[lightVertexIndex], renderPosition,
						scalePosition({ 1.0f, 1.0f })
					); 

					// Each square is composed of 4 quads, so add 4. 
					lightVertexIndex += 4;
				}
			}
		}
		void renderWorldLightMap(const World& world) {
			if (lighting::fullBrightEnabled)
				return; 

			lightMap.clear(lighting::ambientLightColor); 

			lightVertexIndex = 0; 

			for (int chunkOffset = renderableChunkRange.x; chunkOffset <
				renderableChunkRange.y; chunkOffset++)
			{
				const Chunk* chunk = world.getChunk(chunkOffset);

				if (chunk != nullptr)
					renderChunkLightMap(*chunk, world); 
			}

			lightMap.draw(lightMapVerticies, lightVertexIndex, sf::Quads); 
		}
		void finishWorldRendering() {
			if (!lighting::fullBrightEnabled) {
				lightMap.display(); 
				worldRenderTexture.draw(lightMapSprite, sf::BlendMultiply); 
			}

			worldRenderTexture.display(); 
			window::winmain->draw(worldRenderTextureSprite); 
		}
	}
}