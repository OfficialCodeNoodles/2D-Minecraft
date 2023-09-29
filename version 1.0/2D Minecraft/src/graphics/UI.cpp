#include "../../hdr/graphics/UI.hpp"
#include "../../hdr/world/Generation.hpp"
#include "../../hdr/audio/AudioEnviroment.hpp"

namespace engine {
	namespace render {
		namespace ui {
			RemappingButton::RemappingButton() {
			}			
			RemappingButton::RemappingButton(input::Key key, const std::string& keyName) :
				key(key),
				keyName(keyName)
			{
			}

			sf::Font fonts[numOfFonts];
			gs::Text text; 
			float textBackgroundThickness = 0.0f;
			gs::Color textBackgroundColor = gs::Color::Transparent;
			bool textSharpen = false;
			sf::FloatRect prvsTextBounds;
			bool displayDebugMenu = false;
			bool hideUIMenu = false; 
			std::string fpsString;
			sf::Texture cursorTexture;
			sf::Sprite cursorSprite;
			sf::Texture widgetTextures[numOfWidgetTextures];
			sf::Sprite widgetSprite;

			void prepareText(
				int fontId, gs::Vec2f scale, gs::Color fillColor, 
				float outlineThickness, gs::Color outlineColor, 
				float backgroundThickness, gs::Color backgroundColor,
				bool sharpen)
			{
				const sf::Font& font = fonts[fontId];

				text.setFont(font);
				text.setScale(scale);
				text.setFillColor(fillColor);
				text.setOutlineThickness(outlineThickness);
				text.setOutlineColor(outlineColor);

				textBackgroundThickness = backgroundThickness; 
				textBackgroundColor = backgroundColor; 
				textSharpen = sharpen; 

				//if (textSharpen)
				//	// Removes scaling filter to make the text more crisp. 
				//	const_cast<sf::Texture&>(font.getTexture(
				//		text.getText().getCharacterSize())).setSmooth(false);
			}
			void renderText(
				const std::string& string, gs::Vec2f position, int fontId,
				bool centered, gs::Vec2f scale, gs::Color fillColor, 
				float outlineThickness, gs::Color outlineColor, 
				float backgroundThickness, gs::Color backgroundColor, 
				bool sharpen)
			{
				prepareText(
					fontId, scale, fillColor, 
					outlineThickness, outlineColor, 
					backgroundThickness, backgroundColor, 
					sharpen
				); 
				renderText(string, position, centered); 
			}
			void renderText(const std::string& string, gs::Vec2f position,
				bool centered, bool ignoreRendering)
			{
				static sf::RectangleShape textBackground; 

				text.setString(string);
				text.setPosition(
					// Rounds down components to prevent excess blur. 
					std::floor(position.x), std::floor(position.y)
				);

				sf::FloatRect textBounds = text.getText().getGlobalBounds(); 

				// Moves text to position it fits perfectly with the requested
				// position. 
				text.move(
					position.x - textBounds.left, position.y - textBounds.top
				); 

				if (centered)
					text.move(
						-textBounds.width * 0.5f, -textBounds.height * 0.5f
					); 

				if (ignoreRendering) {
					prvsTextBounds = textBounds;
					return;
				}

				if (textBackgroundColor != gs::Color::Transparent) {
					textBounds = text.getText().getGlobalBounds(); 

					textBackground.setSize(gs::Vec2f(
						// Makes the width and height twice the thickness to
						// account for both sides of the background. 
						textBounds.width + (textBackgroundThickness * 2.0f),
						textBounds.height + (textBackgroundThickness * 2.0f)
					)); 
					textBackground.setPosition(gs::Vec2f(
						// Centers the text. 
						textBounds.left - textBackgroundThickness,
						textBounds.top - textBackgroundThickness
					)); 
					textBackground.setFillColor(textBackgroundColor); 

					window::winmain->draw(textBackground); 
				}

				gs::draw(window::winmain, text); 

				prvsTextBounds = textBounds; 
			}
			void prepareMenuButton(gs::Button& button) {
				button.clear(); 
				button.setShapeColors(gs::Color::Transparent); 
				button.setFont(fonts[0]); 
				button.setTextScale(1.5f, 1.5f);
				button.setTextOffset(0.0f, -15.0f); 
				button.setTextShadow(true);
				button.setTextShadowOffset(0.0f, 4.0f); 
			}
			void renderMenuButton(
				gs::Button& button, bool empty, bool keepWhiteText) 
			{
				const float buttonScale = 4.0f; 
				const int buttonEdgeWidth = 2; 

				auto renderButtonBody = [&](gs::Vec2f position, float width) {
					const float buttonTextureWidth = 
						(widgetSprite.getTexture()->getSize().x 
							- (buttonEdgeWidth * 2)) * buttonScale; 

					sf::IntRect bounds; 

					bounds.height = widgetSprite.getTexture()->getSize().y; 
					bounds.left = buttonEdgeWidth; 
					bounds.top = 0; 

					float widthLeft = width;

					do {
						bounds.width = std::min(widthLeft, buttonTextureWidth) / buttonScale; 

						widgetSprite.setTextureRect(bounds); 
						widgetSprite.setPosition(position); 

						window::winmain->draw(widgetSprite); 

						position.x += bounds.width * buttonScale;

						widthLeft -= buttonTextureWidth; 
					} 
					while (widthLeft > 0.0f); 
				}; 
				auto renderButtonRightSide = [&](gs::Vec2f position) {
					sf::IntRect bounds; 

					bounds.width = buttonEdgeWidth; 
					bounds.height = widgetSprite.getTexture()->getSize().y; 
					bounds.left = widgetSprite.getTexture()->getSize().x 
						- bounds.width;
					bounds.top = 0; 

					widgetSprite.setTextureRect(bounds);
					widgetSprite.setPosition(
						position - gs::Vec2f(bounds.width * buttonScale, 0.0f)
					); 

					window::winmain->draw(widgetSprite); 
				}; 
				auto renderButtonLeftSide = [](gs::Vec2f position) {
					sf::IntRect bounds;

					bounds.width = buttonEdgeWidth;
					bounds.height = widgetSprite.getTexture()->getSize().y;
					bounds.left = 0; 
					bounds.top = 0;

					widgetSprite.setTextureRect(bounds);
					widgetSprite.setPosition(position);

					window::winmain->draw(widgetSprite);
				};

				widgetSprite.setTexture(widgetTextures[
					empty ? 0 : (button.isSelected ? 2 : 1)
				]); 
				widgetSprite.setScale(buttonScale, buttonScale); 

				const float buttonHeight = button.getSize().x; 

				renderButtonBody(button.getPosition(), buttonHeight); 
				renderButtonLeftSide(button.getPosition()); 
				renderButtonRightSide(gs::Vec2f(
					button.getPosition().x + buttonHeight,
					button.getPosition().y 
				)); 

				button.setTextColors(
					empty && !keepWhiteText ? gs::Color(127, 127, 127) 
						: gs::Color::White
				); 

				gs::draw(window::winmain, button);
			}

			void prepareControlRemappingButton(RemappingButton& button) {
				prepareMenuButton(button); 

				button.mappedKey = input::keyMap[button.key]; 
				button.mappedKeyName = input::keyNames[button.mappedKey]; 
			}
			void renderControlRemappingButton(
				RemappingButton& button, bool selected) 
			{
				static gs::Text keyNameText; 
				
				// Handle key name text

				keyNameText.setFont(button.getFont()); 
				keyNameText.setScale(button.getTextScale()); 
				keyNameText.setString(button.keyName); 
				keyNameText.setPosition(button.getPosition()); 
				keyNameText.move(-30.0f - keyNameText.getHitbox().getSize().x, 0.0f); 

				gs::draw(window::winmain, keyNameText); 

				// Handle button

				button.setString(
					selected ? ">" + button.mappedKeyName + "<"
						: button.mappedKeyName
				);

				renderMenuButton(button); 
			}
			void prepareMenuSlider(gs::Button& button, gs::Slider& slider) {
				button.clear();
				button.setShapeColors(gs::Color::Transparent);
				button.setFont(fonts[0]);
				button.setTextScale(1.5f, 1.5f);
				button.setTextOffset(0.0f, -15.0f);
				button.setTextShadow(true);
				button.setTextShadowOffset(0.0f, 4.0f);
				button.setTexture(widgetTextures[0]); 
				button.eventTriggerer = gs::Button::EventTriggerer::None; 

				slider.setOnColor(gs::Color::Transparent); 
				slider.setOffColor(gs::Color::Transparent); 
				slider.setOnOutlineThickness(0.0f);
				slider.setOffOutlineThickness(0.0f); 
				slider.button.clear(); 
				slider.button.setSize(28.0f, 80.0f);
			}
			void renderMenuSlider(gs::Button& button, gs::Slider& slider) {
				slider.button.setTexture(widgetTextures[
					slider.isSelected ? 4 : 3
				]);

				renderMenuButton(button, true, true); 
				gs::draw(window::winmain, slider); 
			}
			
			const gs::Vec2f worldPreviewSize = gs::Vec2f(1080.0f, 150.0f); 
			const gs::Vec2i worldPreviewIconSize = gs::Vec2i(128, 128); 
			const int maxWorldNameLength = 28; 
			const float tileSelectionOutlineThickness = 3.0f; 
			const gs::Vec2f inventorySlotSize = gs::Vec2f(16.0f, 16.0f); 

			float guiScale = 4.5f; 
			sf::RectangleShape screenShade;
			float screenShadePercentage = 0.0f;
			float titleIntroTransparencyPercentage = 0.0f; 
			GameTime titleGameTime;
			int titleBackgroundIndex = 0;
			sf::Texture titleIntroLogoTexture;
			sf::Sprite titleIntroLogoSprite; 
			sf::RectangleShape titleIntroProgressBar;
			sf::RectangleShape titleIntroProgressBarOutline;
			sf::Text tileIntroLoadingText;
			sf::Texture titleLogoTexture;
			sf::Sprite titleLogoSprite;
			sf::Texture titleBlurTexture;
			sf::Sprite titleBlurSprite;
			std::vector<std::string> titleSplashTexts;
			gs::Text titleSplashText;
			int titleSplashIndex = 0; 
			gs::Button titleSinglePlayerButton;
			gs::Button titleChangeLogButton;
			gs::Button titleCreditsButton;
			gs::Button titleOptionsButton;
			gs::Button titleQuitGameButton;
			gs::Button titleYouTubeButton;
			sf::RectangleShape titleYouTubeUnderline;
			sf::Texture optionsBackgroundTexture;
			sf::Texture missingWorldTexture;
			sf::Sprite missingWorldSprite;
			sf::Sprite optionsBackgroundSprite;
			gs::Button worldSelectionPlayButton;
			gs::Button worldSelectionEditButton;
			gs::Button worldSelectionDeleteButton;
			gs::Button worldSelectionCreateButton;
			gs::Button worldSelectionCancelButton;
			gs::Slider worldSelectionSlider;
			float worldPreviewOffset;
			sf::RectangleShape worldPreviewOutline;
			int worldPreviewSelected = -1; 
			gs::Textbox worldEditNameTextbox;
			gs::Button worldEditResetIconButton;
			gs::Button worldEditCancelButton;
			gs::Button worldEditSaveButton;
			gs::Textbox worldCreateWorldNameTextbox;
			gs::Textbox worldCreateWorldSeedTextbox;
			gs::Button worldCreateNewWorldButton;
			gs::Button worldCreateCancelButton;
			gs::Button changeLogDoneButton;
			SettingsState settingsState = SettingsState::Main; 
			gs::Button settingsDoneButton;
			gs::Button settingsMusicVolumeButton;
			gs::Slider settingsMusicVolumeSlider;
			gs::Button settingsSoundVolumeButton;
			gs::Slider settingsSoundVolumeSlider;
			gs::Button settingsVideoSettingsButton;
			gs::Button settingsControlsButton;
			gs::Button settingsVideoSettingsFramerateButton;
			gs::Button settingsVideoSettingsLightingStyleButton;
			gs::Button settingsVideoSettingsRenderBiomeBackgroundButton;
			gs::Button settingsVideoSettingsRenderSunAndMoonButton;
			gs::Button settingsVideoSettingsRenderParticlesButton;
			gs::Button settingsVideoSettingsRenderStarsButton;
			gs::Button settingsVideoSettingsGuiScaleButton;
			gs::Slider settingsVideoSettingsGuiScaleSlider;
			gs::Button settingsVideoSettingsDoneButton;
			int remappingButtonSelected = -1; 
			RemappingButton settingsControlsRemappingButtons[numOfRemappingButtons] = {
				{ input::Key::MoveLeft, "Move Left" },
				{ input::Key::MoveRight, "Move Right" }, 
				{ input::Key::Jump, "Jump" },
				{ input::Key::Crouch, "Crouch" },
				{ input::Key::DropItem, "Drop Item" },
				{ input::Key::SwitchTileMode, "Switch Block/Wall Mode" },
				{ input::Key::OpenInventory, "Open Inventory" },
				{ input::Key::MenuEscape, "Close Menu" },
				{ input::Key::HideUI, "Hide UI" }, 
				{ input::Key::TakeScreenshot, "Take Screenshot" },
				{ input::Key::OpenDebugMenu, "Open Debug Menu" }
			}; 
			gs::Slider settingsControlsSlider;
			gs::Button settingsControlsResetKeysButton;
			gs::Button settingsControlsDoneButton;
			sf::Texture itemAtlas;
			sf::Sprite itemSprite;
			Block::Id debugBlockId;
			Wall::Id debugWallId;
			bool useDebugBlock = true; 
			bool gamePaused = false;
			bool pausedSettingsOpen = false; 
			gs::Button pauseBackToGameButton;
			gs::Button pauseSettingsButton;
			gs::Button pauseSaveAndQuitButton;
			gs::Button gameOverRespawnButton;
			gs::Button gameOverTitleMenuButton;
			bool shouldCrosshairBeRendered = false; 
			sf::RectangleShape tileSelectionOutline;
			bool useBlock = true; 
			int hotbarIndex = 0,
				prvsHotbarIndex = 0;
			sf::Texture hotbarTexture;
			sf::Sprite hotbarSprite;
			sf::Texture hotbarSelectorTexture;
			sf::Sprite hotbarSelectorSprite;
			sf::Texture xpBarTextures[numOfXpBarTextures];
			sf::Sprite xpBarSprite;
			sf::Texture heartTextures[numOfHeartTextures];
			sf::Sprite heartSprite;
			sf::Texture hungerTextures[numOfHungerTextures];
			sf::Sprite hungerSprite;
			sf::Texture switcherTextures[numOfSwitcherTextures];
			sf::Sprite switcherSprite;
			InventoryMenu inventoryMenu = InventoryMenu::Inventory; 
			bool inventoryOpen = false, prvsInventoryOpen = false;
			bool gameOver = false;	 
			ItemContainer temporaryItemContainer; 
			sf::Texture containerTextures[numOfContainerTextures];
			sf::Sprite containerSprite;
			sf::Texture fuelProgressTexture;
			sf::Sprite fuelProgressSprite;
			sf::Texture smeltingProgressTexture;
			sf::Sprite smeltingProgressSprite;
			std::string itemPreviewString;
			int inventorySlotSelected = -1;
			std::string selectedItemName;
			float selectedItemNameTimeRemaining = 0.0f; 
			int lastInventorySlotSelected = -1; 
			gs::Vec2f inventorySlotShadePosition;
			sf::RectangleShape inventorySlotShade;
			ItemContainer craftingItemGrid[numOfCraftingGridSquares];
			int craftingSlotSelected = -1;
			ItemContainer craftingOutput;
			bool craftingOutputSelected = false;
			RecipeContainer outputRecipeContainer;
			int furnaceSlotSelected = -1;
			bool furnaceOutputSelected = false;
			int chestSlotSelected = -1;

			void loadAssets() {
				// Load in font files. 
				for (int fontIndex = 0; fontIndex < numOfFonts; fontIndex++) {
					const std::string& fileName = assetDirectory + "fonts/font"
						+ toString(fontIndex);

					// Tries to load in fonts with multiple formats. 
					for (auto& extension : { ".otf", ".ttf" }) {
						if (fonts[fontIndex].loadFromFile(fileName + extension))
							break; 
					}
				}

				// Load in cursor. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/cursor.png", &cursorTexture, 
					&cursorSprite
				)); 

				// Load in widgets. 
				for (int widgetTextureIndex = 0; widgetTextureIndex <
					numOfWidgetTextures; widgetTextureIndex++)
				{
					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, assetDirectory
							+ "textures/gui/widgets/widget" 
							+ toString(widgetTextureIndex) + ".png",
						&widgetTextures[widgetTextureIndex]
					));
				}

				// Load in titleIntroLogo. 
				if (titleIntroLogoTexture.loadFromFile(assetDirectory
						+ "textures/gui/titleIntroLogo.png"))
					titleIntroLogoSprite.setTexture(titleIntroLogoTexture);

				// Initialize the titleIntroLoadingText. 
				tileIntroLoadingText.setFont(fonts[0]); 

				// Initialize the titleIntroLoadingBar. 

				const float titleIntroLoadingBarOutlineThickness = 4.0f;

				titleIntroProgressBarOutline.setSize(gs::Vec2f(900.0f, 35.0f));
				titleIntroProgressBarOutline.setPosition(gs::Vec2f(
					(window::defaultWindowWidth / 2.0f)
						- (titleIntroProgressBarOutline.getSize().x / 2.0f),
					820.0f
				));
				titleIntroProgressBarOutline.setFillColor(gs::Color::Transparent);
				titleIntroProgressBarOutline.setOutlineThickness(
					titleIntroLoadingBarOutlineThickness
				);
				titleIntroProgressBarOutline.setOutlineColor(gs::Color::White);

				titleIntroProgressBar.setSize(gs::Vec2f(0.0f,
					titleIntroProgressBarOutline.getSize().y
						- (titleIntroLoadingBarOutlineThickness * 2.0f)
				));
				titleIntroProgressBar.setPosition(gs::Vec2f(
					titleIntroProgressBarOutline.getPosition().x
						+ (titleIntroLoadingBarOutlineThickness * 1.0f),
					titleIntroProgressBarOutline.getPosition().y
						+ (titleIntroLoadingBarOutlineThickness * 1.0f)
				));

				// Load in titleLogo. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/titleLogo.png", &titleLogoTexture,
					&titleLogoSprite
				));

				// Load in titleBlur. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/titleBlur.png", &titleBlurTexture, 
					&titleBlurSprite
				));

				// Load in splashTexts. 
				
				const PairVector& pairs = loadPairedFile(
					assetDirectory + "data/splashTexts.list", ';', "End", false
				); 

				for (auto& [phrase, seperator] : pairs) 
					titleSplashTexts.push_back(phrase); 

				// Load in optionsBackground. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory 
						+ "textures/gui/optionsBackground.png", 
					&optionsBackgroundTexture, &optionsBackgroundSprite 
				));

				// Load in missingWorldIcon. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/missingWorldIcon.png", 
					&missingWorldTexture, &missingWorldSprite
				)); 

				// Load in item-atlas. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/atlases/item.png", &itemAtlas
				));

				// Load in hotbar. 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/hotbar.png", &hotbarTexture, 
					&hotbarSprite
				)); 
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/hotbarSelector.png",
					&hotbarSelectorTexture, &hotbarSelectorSprite
				));

				// Load in xp-bar textures
				for (int xpBarTextureIndex = 0; xpBarTextureIndex <
					numOfXpBarTextures; xpBarTextureIndex++)
				{
					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, assetDirectory
							+ "textures/gui/xpBar/xpBar"
							+ toString(xpBarTextureIndex) + ".png",
						&xpBarTextures[xpBarTextureIndex]
					)); 
				}

				// Load in heart textures
				for (int heartTextureIndex = 0; heartTextureIndex <
					numOfHeartTextures; heartTextureIndex++)
				{
					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, assetDirectory
							+ "textures/gui/hearts/heart"
							+ toString(heartTextureIndex) + ".png", 
						&heartTextures[heartTextureIndex]
					)); 
				}

				// Load in hunger textures
				for (int hungerTextureIndex = 0; hungerTextureIndex <
					numOfHungerTextures; hungerTextureIndex++)
				{
					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, assetDirectory 
							+ "textures/gui/hunger/hunger"
							+ toString(hungerTextureIndex) + ".png",
						&hungerTextures[hungerTextureIndex]
					)); 
				}

				// Load in switcher textures
				for (int switcherTextureIndex = 0; switcherTextureIndex <
					numOfSwitcherTextures; switcherTextureIndex++)
				{
					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, assetDirectory
							+ "textures/gui/switcher/switcher"
							+ toString(switcherTextureIndex) + ".png",
						&switcherTextures[switcherTextureIndex]
					));
				}

				// Load in container textures
				for (int containerTextureIndex = 0; containerTextureIndex <
					numOfContainerTextures; containerTextureIndex++)
				{
					addResourceLoader(ResourceLoader(
						ResourceLoader::Type::Texture, assetDirectory
							+ "textures/gui/container/container"
							+ toString(containerTextureIndex) + ".png",
						&containerTextures[containerTextureIndex]
					)); 
				}

				// Load in furnace textures
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/container/fuelProgress.png",
					&fuelProgressTexture, &fuelProgressSprite
				));
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::Texture, assetDirectory
						+ "textures/gui/container/smeltingProgress.png",
					&smeltingProgressTexture, &smeltingProgressSprite
				)); 
			}
			void handleAssets() {
				// Initialize the cursorSprite. 
				cursorSprite.setOrigin(
					static_cast<float>(cursorTexture.getSize().x) / 2.0f,
					static_cast<float>(cursorTexture.getSize().y) / 2.0f
				);
				cursorSprite.setScale(3.0f, 3.0f);

				// Initialize titleBackgroundIndex. 
				titleBackgroundIndex = randomGenerator.generate() % Biome::End;

				// Initialize the titleGameTime.
				titleGameTime.gameTicks = randomGenerator.generate() 
					% GameTime::maxTime;

				// Initialize the titleLogo. 
				titleLogoSprite.setScale(1.0f, 1.0f); 
				titleLogoSprite.setPosition(
					(window::defaultWindowWidth / 2.0f), 200.0f
				);
				titleLogoSprite.setOrigin(
					titleLogoTexture.getSize().x 
						* titleLogoSprite.getScale().x * 0.5f, 
					titleLogoTexture.getSize().y
						* titleLogoSprite.getScale().y * 0.5f
				); 

				// Initialize titleSplashIndex. 
				titleSplashIndex = randomGenerator.generate() 
					% titleSplashTexts.size();

				// Initialize the titleSplashText. 
				titleSplashText.setFont(fonts[0]); 
				titleSplashText.setFillColor(gs::Color::Yellow); 
				titleSplashText.setShadow(true); 
				titleSplashText.setShadowColor(gs::Color(127, 127, 0)); 

				// Initialize title buttons. 
				
				const float buttonHeight = 80.0f; 
				const float medialButtonWidth = 400.0f; 
				const float medialButtonGap = 8.0f; 

				titleSinglePlayerButton.setSize(
					(medialButtonWidth * 2.0f) + (medialButtonGap * 2.0f),
					buttonHeight
				); 
				titleSinglePlayerButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						- (titleSinglePlayerButton.getSize().x / 2.0f),
					490.0f
				);
				titleSinglePlayerButton.setString("Singleplayer");
				prepareMenuButton(titleSinglePlayerButton); 

				titleChangeLogButton = titleSinglePlayerButton; 
				titleChangeLogButton.setString("Change Log"); 
				titleChangeLogButton.move(0.0f, 100.0f); 

				titleCreditsButton = titleChangeLogButton; 
				titleCreditsButton.setString("Credits");
				titleCreditsButton.move(0.0f, 100.0f); 

				titleOptionsButton.setSize(medialButtonWidth, buttonHeight); 
				titleOptionsButton.setPosition(
					(window::defaultWindowWidth / 2.0f) - medialButtonGap 
						- medialButtonWidth, 830.0f
				);
				titleOptionsButton.setString("Options..."); 
				prepareMenuButton(titleOptionsButton); 

				titleQuitGameButton.setSize(medialButtonWidth, buttonHeight);
				titleQuitGameButton.setPosition(
					(window::defaultWindowWidth / 2.0f) + medialButtonGap, 830.0f
				); 
				titleQuitGameButton.setString("Quit Game"); 
				prepareMenuButton(titleQuitGameButton); 

				titleYouTubeButton.setSize(480.0f, 40.0f);
				titleYouTubeButton.setPosition(1435.0f, 1030.0f); 
				titleYouTubeButton.setShapeColors(gs::Color::Transparent); 
				titleYouTubeButton.setScaleModifiers(1.0f); 
				titleYouTubeButton.setFont(fonts[0]); 
				titleYouTubeButton.setString("Check out my YouTube channel!"); 
				titleYouTubeButton.setTextColors(gs::Color::Cyan); 
				titleYouTubeButton.setSelectedTextFillColor(gs::Color::Blue); 
				titleYouTubeButton.setColorAdjustSpeed(100.0f); 

				titleYouTubeUnderline.setSize(
					gs::Vec2f(titleYouTubeButton.getSize().x, 3.0f)
				); 
				titleYouTubeUnderline.setPosition(
					titleYouTubeButton.getPosition().x,
					titleYouTubeButton.getPosition().y
						+ titleYouTubeButton.getSize().y + 5.0f
				);

				// Initialize the optionsBackground. 
				optionsBackgroundTexture.setRepeated(true);
				optionsBackgroundSprite.setScale(6.0f, 6.0f);

				// Initialize the world selection components. 

				const float selectionButtonWidth = 600.0f; 

				worldSelectionPlayButton.setSize(selectionButtonWidth, buttonHeight); 
				worldSelectionPlayButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						- (medialButtonGap * 2.0f) - selectionButtonWidth, 880.0f
				); 
				worldSelectionPlayButton.setString("Play Selected World"); 
				prepareMenuButton(worldSelectionPlayButton); 

				worldSelectionEditButton = worldSelectionPlayButton;  
				worldSelectionEditButton.setSize(
					(selectionButtonWidth / 2.0f) - medialButtonGap,
					buttonHeight
				); 
				worldSelectionEditButton.setString("Edit"); 
				worldSelectionEditButton.move(0.0f, 90.0f); 

				worldSelectionDeleteButton = worldSelectionEditButton; 
				worldSelectionDeleteButton.setString("Delete"); 
				worldSelectionDeleteButton.move(
					worldSelectionDeleteButton.getSize().x 
						+ (medialButtonGap * 2), 0.0f
				);

				worldSelectionCreateButton.setSize(selectionButtonWidth, buttonHeight); 
				worldSelectionCreateButton.setPosition(
					(window::defaultWindowWidth / 2.0f) 
						+ (medialButtonGap * 2.0f), 880.0f
				);
				worldSelectionCreateButton.setString("Create New World"); 
				prepareMenuButton(worldSelectionCreateButton); 

				worldSelectionCancelButton = worldSelectionCreateButton; 
				worldSelectionCancelButton.setString("Cancel"); 
				worldSelectionCancelButton.move(0.0f, 90.0f);

				const float sliderOffsetScaler = 
					optionsBackgroundTexture.getSize().y
						* optionsBackgroundSprite.getScale().y; 

				worldSelectionSlider.setSize(
					20.0f, window::defaultWindowHeight 
						- (4.5f * sliderOffsetScaler)
				); 
				worldSelectionSlider.setPosition(1500.0f, 2.0f * sliderOffsetScaler); 
				worldSelectionSlider.setOffColor(gs::Color::Black); 
				worldSelectionSlider.setOnColor(gs::Color::Black); 
				worldSelectionSlider.button.setSize(20.0f, 100.0f); 
				worldSelectionSlider.button.clear(); 
				worldSelectionSlider.button.setSelectedFillColor(gs::Color(180, 180, 180)); 
				worldSelectionSlider.button.setClickedFillColor(
					worldSelectionSlider.button.getSelectedFillColor()
				); 

				worldPreviewOutline.setSize(worldPreviewSize + gs::Vec2f(0.0f, -12.0f)); 
				worldPreviewOutline.setFillColor(gs::Color::Transparent); 
				worldPreviewOutline.setOutlineThickness(5.0f); 
				worldPreviewOutline.setOutlineColor(gs::Color::White); 

				// Initialize the world edit components. 

				worldEditNameTextbox.setSize(800.0f, 80.0f); 
				worldEditNameTextbox.setCenter(window::defaultWindowWidth / 2.0f, 220.0f); 
				worldEditNameTextbox.setScaleModifiers(1.0f); 
				worldEditNameTextbox.setColors(gs::Color::Black); 
				worldEditNameTextbox.setOutlineThickness(5.0f);
				worldEditNameTextbox.setFont(fonts[0]); 
				worldEditNameTextbox.setTextColors(gs::Color::White); 
				worldEditNameTextbox.setTextScale(1.5f, 1.5f);
				worldEditNameTextbox.setTextOffset(22.0f, -12.0f); 
				worldEditNameTextbox.setTextShadow(true); 
				worldEditNameTextbox.setMaxInputLength(maxWorldNameLength); 
				worldEditNameTextbox.textOrigin = gs::Textbox::TextOrigin::Left; 

				worldEditResetIconButton.setSize(
					800.0f + (medialButtonGap * 2.0f), buttonHeight
				); 
				worldEditResetIconButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						- (worldEditResetIconButton.getSize().x / 2.0f), 
					300.0f
				); 
				worldEditResetIconButton.setString("Reset Icon"); 
				prepareMenuButton(worldEditResetIconButton); 

				worldEditCancelButton.setSize(400.0f, buttonHeight); 
				worldEditCancelButton.setPosition(
					(window::defaultWindowWidth / 2.0f) + medialButtonGap,
					880.0f
				); 
				worldEditCancelButton.setString("Cancel"); 
				prepareMenuButton(worldEditCancelButton); 

				worldEditSaveButton = worldEditCancelButton; 
				worldEditSaveButton.setString("Save"); 
				worldEditSaveButton.move(
					-worldEditSaveButton.getSize().x
						- (medialButtonGap * 2.0f), 0.0f
				); 

				// Initialize the world create components; 

				worldCreateWorldNameTextbox = worldEditNameTextbox; 

				worldCreateWorldSeedTextbox = worldCreateWorldNameTextbox; 
				worldCreateWorldSeedTextbox.move(0.0f, 160.0f); 
				worldCreateWorldSeedTextbox.validInputs = 
					gs::Textbox::ValidInputs::Numeric; 
				worldCreateWorldSeedTextbox.setMaxInputLength(6);
				worldCreateWorldSeedTextbox.setMessageString("Leave empty for random seed");

				worldCreateCancelButton.setSize(
					selectionButtonWidth, buttonHeight
				); 
				worldCreateCancelButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						+ (medialButtonGap * 2.0f), 900.0f
				); 
				worldCreateCancelButton.setString("Cancel"); 
				prepareMenuButton(worldCreateCancelButton); 

				worldCreateNewWorldButton = worldCreateCancelButton; 
				worldCreateNewWorldButton.setString("Create New World"); 
				worldCreateNewWorldButton.move(
					-worldCreateNewWorldButton.getSize().x
						- (medialButtonGap * 4.0f), 0.0f
				);

				// Initialize the change-log menu components. 

				changeLogDoneButton.setSize(600.0f, buttonHeight); 
				changeLogDoneButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						- (changeLogDoneButton.getSize().x / 2.0f), 900.0f
				); 
				changeLogDoneButton.setString("Back"); 
				prepareMenuButton(changeLogDoneButton); 

				// Initialize the settings menu components. 

				settingsDoneButton.setSize(600.0f, buttonHeight); 
				settingsDoneButton.setPosition(
					(window::defaultWindowWidth / 2.0f) 
						- (settingsDoneButton.getSize().x / 2.0f), 700.0f
				);
				settingsDoneButton.setString("Done"); 
				prepareMenuButton(settingsDoneButton); 

				settingsMusicVolumeButton.setSize(600.0f, buttonHeight); 
				settingsMusicVolumeButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						-(settingsMusicVolumeButton.getSize().x)
							-(medialButtonGap * 2.0f), 200.0f
				);
				settingsMusicVolumeButton.setString(
					"Music: " + toString(static_cast<int>(audio::musicVolume)) 
						+ "%"
				);

				settingsMusicVolumeSlider.setSize(
					settingsMusicVolumeButton.getSize()
				); 
				settingsMusicVolumeSlider.setPosition(
					settingsMusicVolumeButton.getPosition()
				); 
				prepareMenuSlider(settingsMusicVolumeButton, 
					settingsMusicVolumeSlider); 
				settingsMusicVolumeSlider.setPercentage(audio::musicVolume);
				settingsMusicVolumeSlider.update(); 

				settingsSoundVolumeButton = settingsMusicVolumeButton;
				settingsSoundVolumeButton.move(
					(settingsSoundVolumeButton.getSize().x * 1.0f)
						+ (medialButtonGap * 4.0f), 0.0f
				); 
				settingsSoundVolumeButton.setString(
					"Sound: " + toString(static_cast<int>(audio::soundVolume)) 
						+ "%"
				);

				settingsSoundVolumeSlider.setSize(
					settingsSoundVolumeButton.getSize()
				); 
				settingsSoundVolumeSlider.setPosition(
					settingsSoundVolumeButton.getPosition()
				); 
				prepareMenuSlider(settingsSoundVolumeButton, 
					settingsSoundVolumeSlider); 
				settingsSoundVolumeSlider.setPercentage(audio::soundVolume);
				settingsSoundVolumeSlider.update(); 

				settingsVideoSettingsButton.setSize(
					settingsMusicVolumeButton.getSize()
				); 
				settingsVideoSettingsButton.setPosition(
					settingsMusicVolumeButton.getPosition()
				); 
				settingsVideoSettingsButton.move(0.0f, 380.0f); 
				settingsVideoSettingsButton.setString("Video Settings..."); 
				prepareMenuButton(settingsVideoSettingsButton); 

				settingsControlsButton.setSize(
					settingsSoundVolumeButton.getSize()
				);
				settingsControlsButton.setPosition(
					settingsSoundVolumeButton.getPosition()
				);
				settingsControlsButton.move(0.0f, 380.0f); 
				settingsControlsButton.setString("Controls...");
				prepareMenuButton(settingsControlsButton); 

				settingsVideoSettingsFramerateButton = settingsVideoSettingsButton; 
				settingsVideoSettingsFramerateButton.setPosition(
					settingsMusicVolumeButton.getPosition()
				); 
				settingsVideoSettingsFramerateButton.setString(
					"Framerate: " + toString(
						render::window::renderingFramerate
					) + " fps"
				); 

				settingsVideoSettingsLightingStyleButton = settingsVideoSettingsButton; 
				settingsVideoSettingsLightingStyleButton.setPosition(
					settingsSoundVolumeButton.getPosition()
				);
				settingsVideoSettingsLightingStyleButton.setString(
					"Lighting Style: " + std::string(render::lighting::lightingStyle ==
						render::lighting::LightingStyle::Geometry
							? "Geometry" : "Smooth")
				);

				settingsVideoSettingsRenderBiomeBackgroundButton = settingsVideoSettingsButton; 
				settingsVideoSettingsRenderBiomeBackgroundButton.setPosition(
					settingsMusicVolumeButton.getPosition()
				); 
				settingsVideoSettingsRenderBiomeBackgroundButton.move(0.0f, 100.0f); 
				settingsVideoSettingsRenderBiomeBackgroundButton.setTextScale(1.2f, 1.2f); 
				settingsVideoSettingsRenderBiomeBackgroundButton.setTextOffset(0.0f, -12.0f); 
				settingsVideoSettingsRenderBiomeBackgroundButton.setString(
					"Render Biome Background: " + std::string(
						shouldBiomeBackgroundBeRendered ? "Yes" : "No")
				);

				settingsVideoSettingsRenderSunAndMoonButton = settingsVideoSettingsButton; 
				settingsVideoSettingsRenderSunAndMoonButton.setPosition(
					settingsSoundVolumeButton.getPosition()
				); 
				settingsVideoSettingsRenderSunAndMoonButton.move(0.0f, 100.0f); 
				settingsVideoSettingsRenderSunAndMoonButton.setString(
					"Render Sun & Moon: " + std::string(
						shouldSunAndMoonBeRendered ? "Yes" : "No")
				);

				settingsVideoSettingsRenderParticlesButton = settingsVideoSettingsButton; 
				settingsVideoSettingsRenderParticlesButton.setPosition(
					settingsMusicVolumeButton.getPosition()
				); 
				settingsVideoSettingsRenderParticlesButton.move(0.0f, 200.0f); 
				settingsVideoSettingsRenderParticlesButton.setString(
					"Render Particles: " + std::string(
						shouldParticlesBeRendered ? "Yes" : "No")
				); 

				settingsVideoSettingsRenderStarsButton = settingsVideoSettingsButton; 
				settingsVideoSettingsRenderStarsButton.setPosition(
					settingsSoundVolumeButton.getPosition()
				); 
				settingsVideoSettingsRenderStarsButton.move(0.0f, 200.0f); 
				settingsVideoSettingsRenderStarsButton.setString(
					"Render Stars: " + std::string(
						shouldStarsBeRendered ? "Yes" : "No")
				); 

				settingsVideoSettingsGuiScaleButton = settingsMusicVolumeButton;
				settingsVideoSettingsGuiScaleButton.move(0.0f, 300.0f);
				settingsVideoSettingsGuiScaleButton.setString(
					"Gui Scale: " + toString(guiScale).substr(0, 
						toString(guiScale).find('.') + 2)
				);

				settingsVideoSettingsGuiScaleSlider.setSize(
					settingsVideoSettingsGuiScaleButton.getSize()
				);
				settingsVideoSettingsGuiScaleSlider.setPosition(
					settingsVideoSettingsGuiScaleButton.getPosition()
				);
				prepareMenuSlider(settingsVideoSettingsGuiScaleButton,
					settingsVideoSettingsGuiScaleSlider);
				settingsVideoSettingsGuiScaleSlider.setPercentage(
					guiScale - 1.0f, 9.0f
				);
				settingsVideoSettingsGuiScaleButton.update();

				settingsVideoSettingsDoneButton = settingsDoneButton;  

				for (int remappingButtonIndex = 0; remappingButtonIndex <
					numOfRemappingButtons; remappingButtonIndex++)
				{
					RemappingButton& remappingButton =
						settingsControlsRemappingButtons[remappingButtonIndex];

					remappingButton.setSize(300.0f, buttonHeight); 
					remappingButton.setPosition(
						(window::defaultWindowWidth / 2.0f)
							+ (medialButtonGap * 2.0f),
						225.0f + (remappingButtonIndex * 100.0f)
					);
					prepareControlRemappingButton(remappingButton);
				}

				settingsControlsSlider = worldSelectionSlider; 

				settingsControlsResetKeysButton = settingsDoneButton;
				settingsControlsResetKeysButton.setString("Reset Keybindings"); 
				settingsControlsResetKeysButton.move(
					settingsDoneButton.getSize().x * 0.5f
						+ medialButtonGap, 200.0f
				);

				settingsControlsDoneButton = settingsDoneButton; 
				settingsControlsDoneButton.move(
					settingsDoneButton.getSize().x * -0.5f 
						- medialButtonGap, 200.0f
				);
					
				// Initialize the pause menu components. 

				pauseBackToGameButton.setSize(800.0f, buttonHeight); 
				pauseBackToGameButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						- (pauseBackToGameButton.getSize().x / 2.0f), 480.0f
				);
				pauseBackToGameButton.setString("Back to Game");
				prepareMenuButton(pauseBackToGameButton);

				pauseSettingsButton = pauseBackToGameButton; 
				pauseSettingsButton.setString("Options"); 
				pauseSettingsButton.move(0.0f, 100.0f); 

				pauseSaveAndQuitButton = pauseSettingsButton; 
				pauseSaveAndQuitButton.setString("Save and Quit to Title"); 
				pauseSaveAndQuitButton.move(0.0f, 100.0f); 

				// Initialize the game over menu components. 

				gameOverRespawnButton.setSize(800.0f, buttonHeight);
				gameOverRespawnButton.setPosition(
					(window::defaultWindowWidth / 2.0f)
						- (gameOverRespawnButton.getSize().x / 2.0f), 530.0f
				); 
				gameOverRespawnButton.setString("Respawn"); 
				prepareMenuButton(gameOverRespawnButton); 

				gameOverTitleMenuButton = gameOverRespawnButton; 
				gameOverTitleMenuButton.setString("Title menu"); 
				gameOverTitleMenuButton.move(0.0f, 100.0f); 

				// Initialize the hotbarSelectorSprite. 
				hotbarSelectorSprite.setOrigin(
					hotbarSelectorTexture.getSize().x / 2.0f,
					hotbarSelectorTexture.getSize().y / 2.0f
				);

				// Initialize the screenShade box. 
				screenShade.setSize(gs::Vec2f(window::defaultWindowWidth, 
					window::defaultWindowHeight)); 

				// Initialize the tileSelectionOutline box.
				tileSelectionOutline.setFillColor(gs::Color::Transparent); 
				tileSelectionOutline.setOutlineThickness(
					tileSelectionOutlineThickness
				); 
				tileSelectionOutline.setOutlineColor(gs::Color::Black); 

				// Initialize the xpBarSprite. 
				xpBarSprite.setTexture(xpBarTextures[0]); 
				xpBarSprite.setOrigin(
					xpBarSprite.getTexture()->getSize().x / 2.0f,
					xpBarSprite.getTexture()->getSize().y
				); 

				// Initialize the heartSprite. 
				heartSprite.setTexture(heartTextures[0]);
				heartSprite.setOrigin(
					heartSprite.getTexture()->getSize().x / 2.0f,
					heartSprite.getTexture()->getSize().y
				); 

				// Initialize the hungerSprite. 
				hungerSprite.setTexture(hungerTextures[0]); 
				hungerSprite.setOrigin(
					hungerSprite.getTexture()->getSize().x / 2.0f,
					hungerSprite.getTexture()->getSize().y
				);  
				
				// Initialize the temoraryItemContainer. 
				temporaryItemContainer.count = 0; 

				// Initalize the containerSprite.
				containerSprite.setTexture(containerTextures[0]); 

				// Initialize the inventorySlotShade.
				inventorySlotShade.setSize(inventorySlotSize); 
				inventorySlotShade.setOrigin(inventorySlotShade.getSize() * 0.5f); 
				inventorySlotShade.setFillColor(gs::Color(100, 100, 100)); 
			}
			gs::Vec2f getWorldPreviewPosition(int worldPreviewIndex) {
				return gs::Vec2f(400.0f, 
					230.0f + (worldPreviewIndex * worldPreviewSize.y)
						- worldPreviewOffset
				);
			}
			gs::Vec2f getInventorySlotPosition(int inventorySlotIndex) {
				const float containerSlotGapWidth = 18.0f * guiScale; 
				const gs::Vec2i inventoryPosition = gs::Vec2i(
					inventorySlotIndex % Player::numOfHotbarSlots,
					inventorySlotIndex / Player::numOfHotbarSlots
				);

				gs::Vec2f position = containerSprite.getPosition();

				position.x -= (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y += (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(16.0f, -16.0f) * guiScale;

				position.x += inventoryPosition.x * containerSlotGapWidth;
				position.y -= inventoryPosition.y * containerSlotGapWidth;

				if (inventoryPosition.y > 0)
					position.y -= 4.0f * guiScale;

				return position; 
			}
			gs::Vec2f getCraftingSlotPosition(
				int craftingSlotIndex, bool fullTable) 
			{
				const float containerSlotGapWidth = 18.0f * guiScale; 
				const gs::Vec2i inventoryPosition = gs::Vec2i(
					craftingSlotIndex % (fullTable ? 3 : 2), 
					craftingSlotIndex / (fullTable ? 3 : 2)
				);

				gs::Vec2f position = containerSprite.getPosition(); 

				position.x += (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y -= (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(
					fullTable ? -138.0f : -70.0f, 
					fullTable ? 61.0f : 44.0f
				) * guiScale; 

				position.x += inventoryPosition.x * containerSlotGapWidth;
				position.y -= inventoryPosition.y * containerSlotGapWidth;

				return position; 
			}
			gs::Vec2f getCraftingOutputSlotPosition(bool fullTable) {
				gs::Vec2f position = containerSprite.getPosition();

				position.x += (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y -= (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(
					fullTable ? -44.0f : -14.0f,
					fullTable ? 43.0f : 36.0f
				) * guiScale;

				return position;
			}
			gs::Vec2f getFurnaceInputPosition(int furnaceSlot) {
				gs::Vec2f position = containerSprite.getPosition(); 

				position.x += (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y -= (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(
					-112.0f, 25.0f + (furnaceSlot * 36.0f)
				) * guiScale; 

				return position; 
			}
			gs::Vec2f getFurnaceOutputPosition() {
				gs::Vec2f position = containerSprite.getPosition();

				position.x += (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y -= (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(-52.0f, 43.0f) * guiScale;

				return position;

			}
			gs::Vec2f getFurnaceFuelProgressPosition() {
				gs::Vec2f position = containerSprite.getPosition();

				position.x += (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y -= (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(-120.0f, 36.0f) * guiScale;

				return position;
			}
			gs::Vec2f getFurnaceSmeltingProgressPosition() {
				gs::Vec2f position = containerSprite.getPosition();

				position.x += (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y -= (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(-97.0f, 35.0f) * guiScale;

				return position;
			}
			gs::Vec2f getChestSlotPosition(int chestSlotIndex) {
				const float containerSlotGapWidth = 18.0f * guiScale;
				const gs::Vec2i chestPosition = gs::Vec2i(
					chestSlotIndex % Player::numOfHotbarSlots,
					chestSlotIndex / Player::numOfHotbarSlots
				);

				gs::Vec2f position = containerSprite.getPosition();

				position.x -= (containerSprite.getTexture()->getSize().x / 2.0f) * guiScale;
				position.y += (containerSprite.getTexture()->getSize().y / 2.0f) * guiScale;

				position += gs::Vec2f(16.0f, -104.0f) * guiScale;

				position.x += chestPosition.x * containerSlotGapWidth;
				position.y -= chestPosition.y * containerSlotGapWidth;

				return position;
			}

			void renderTileBreaking() {
				if (tileBreakPercentage > 0.0f) {
					// Set the breaking texture based on how far along the tile
					// is from being broken. 
					tileBreakingStageSprite.setTexture(
						tileBreakingStageTextures[static_cast<int>(
							tileBreakPercentage) / 10]
					);

					tileBreakingStageSprite.setScale(cameraScale, cameraScale);
					tileBreakingStageSprite.setPosition(transformPosition(
						gs::Vec2f(tileBeingBroken.x, tileBeingBroken.y)
					));

					worldRenderTexture.draw(tileBreakingStageSprite, sf::BlendMultiply);
				}
			}

			void renderTilePreview(gs::Vec2i position, sf::Sprite& sprite) {
				const gs::Color originalSpriteColor = sprite.getColor(); 
				const gs::Color previewColor = gs::Color(127, 127, 127, 127); 

				sprite.setPosition(transformPosition(
					gs::Vec2f(position.x, position.y)
				));
				sprite.setScale(cameraScale, cameraScale);
				sprite.setRotation(0.0f);
				sprite.setColor(previewColor);
				
				window::winmain->draw(sprite);
				
				sprite.setColor(originalSpriteColor);
			}
			void renderBlockPreview(Block::Id blockId, gs::Vec2i position) {
				applyTextureBounds(getBlockTextureIndex(Block(blockId)), blockSprite);
				renderTilePreview(position, blockSprite);
			}
			void renderWallPreview(Wall::Id wallId, gs::Vec2i position) {
				applyTextureBounds(getWallTextureIndex(Wall(wallId)), wallSprite);
				renderTilePreview(position, wallSprite);
			}
			void renderItem(
				const Item& item, gs::Vec2f position, float size,
				sf::RenderTarget& target)
			{
				static sf::RectangleShape itemBox;

				const int numOfAtlases = 3;
				// All possible atlases used in item rendering. 
				const sf::Texture* textureAtlases[numOfAtlases] = {
					&itemAtlas, &blockAtlas, &wallAtlas
				};

				const gs::Vec2f renderPosition = position;

				const int textureIndex = item.getVar(ItemInfo::textureIndex);
				const int textureAtlasIndex = std::min(
					textureIndex / ItemInfo::defaultTileItems,
					numOfAtlases - 1
				);

				itemSprite.setTexture(*textureAtlases[textureAtlasIndex]);
				applyTextureBounds(
					textureIndex - (textureAtlasIndex * ItemInfo::defaultTileItems),
					itemSprite
				);

				// Make items rendered from item-atlas slightly bigger. 
				if (textureAtlasIndex == 0)
					size *= 1.4f;

				const float normalizedSize = size / tileSize;

				itemSprite.setScale(
					normalizedSize * gs::util::sign(itemSprite.getScale().x),
					normalizedSize * gs::util::sign(itemSprite.getScale().y)
				);
				itemSprite.setOrigin(tileSize * 0.5f, tileSize * 0.5f);
				itemSprite.setPosition(position);

				target.draw(itemSprite);
			}
			void renderInventoryItem(
				const ItemContainer& itemContainer, gs::Vec2f position)
			{
				static sf::RectangleShape durabilityBar;

				// Ensures items actually exist in the hotbar slot. 
				if (itemContainer.count != 0) {
					const int itemMaxDurability = itemContainer.item.getVar(
						ItemInfo::maxDurability
					);

					renderItem(itemContainer.item, position, 10.0f * guiScale);

					// Only displays text if the number of items is 
					// greater than 1.
					if (itemContainer.count > 1) {
						const std::string displayNumber =
							// Adds space if item count is only 1 digit.
							itemContainer.count < 10 ? " "
							+ toString(itemContainer.count)
							: toString(itemContainer.count);
						const gs::Vec2f textScale = gs::Vec2f(
							guiScale, guiScale) * 0.3f;

						// Reposition text from item center. 
						position.x -= guiScale * 2.0f;
						position.y += guiScale * 2.0f;

						// Render background text. 
						renderText(
							displayNumber, gs::Vec2f(position),
							0, false, textScale, gs::Color::Black, 0.0f,
							gs::Color(), 0.0f, gs::Color::Transparent, true
						);

						position.x -= guiScale * 1.0f;
						position.y -= guiScale * 1.0f;

						// Render foreground text. 
						renderText(
							displayNumber, gs::Vec2f(position),
							0, false, textScale, gs::Color::White, 0.0f,
							gs::Color(), 0.0f, gs::Color::Transparent, true
						);
					}
					else if (itemMaxDurability > 0) {
						const float ratio = itemContainer.item.tags.durability
							/ static_cast<float>(itemMaxDurability); 

						// Only renders the item durability if it is less than
						// the starting durability. 
						if (ratio < 1.0f) {
							// Width in pixels of the bar when full. 
							const float barWidth = 12.0f; 

							// Render the background of the bar. 

							durabilityBar.setSize(
								gs::Vec2f(barWidth + 1.0f, 2.0f) * guiScale
							); 
							durabilityBar.setPosition(
								position + gs::Vec2f(-6.0f, 5.0f) * guiScale
							); 
							durabilityBar.setFillColor(gs::Color::Black); 

							window::winmain->draw(durabilityBar);

							// Render the remaining durability of the bar. 

							durabilityBar.setSize(
								gs::Vec2f(barWidth * ratio, 1.0f) * guiScale
							);
							durabilityBar.setFillColor(gs::util::approach(
								gs::Color::Red, gs::Color::Green, ratio * 100.0f
							)); 

							window::winmain->draw(durabilityBar); 

							// Render empty segment of the bar. 

							durabilityBar.move(durabilityBar.getSize().x, 0.0f); 
							durabilityBar.setSize(gs::Vec2f(
								(barWidth * guiScale) - durabilityBar.getSize().x,
								guiScale)
							); 
							durabilityBar.setFillColor(gs::util::approach(
								durabilityBar.getFillColor(), gs::Color::Black, 80.0f)
							);

							window::winmain->draw(durabilityBar);
						}
					}
				}
			}
			void renderTextBox(const std::string& name, gs::Vec2f position) {
				static sf::RectangleShape background, insideOutline; 

				const float boarderThickness = 2.0f * guiScale; 
				const gs::Vec2f textScale = gs::Vec2f(guiScale, guiScale) * 0.3f;

				prepareText(0, textScale); 
				renderText(name, position + gs::Vec2f(boarderThickness, 
					boarderThickness), false, true);

				const float outlineThickness = 1.0f; 
				const float scaledOutlineThickness = outlineThickness * guiScale; 
				const gs::Vec2f size = gs::Vec2f(
					prvsTextBounds.width + (boarderThickness * 2.0f), 
					prvsTextBounds.height + (boarderThickness * 2.0f)
				); 

				// Render black background. 

				background.setSize(
					size + gs::Vec2f(scaledOutlineThickness * 2.0f,
						scaledOutlineThickness * 4.0f)
				);
				background.setPosition(
					position - gs::Vec2f(scaledOutlineThickness, 
						scaledOutlineThickness * 2.0f)
				); 
				background.setFillColor(gs::Color(0, 0, 0, 180)); 

				window::winmain->draw(background); 

				background.setSize(
					size + gs::Vec2f(scaledOutlineThickness * 4.0f,
						scaledOutlineThickness * 2.0f)
				); 
				background.setPosition(
					position - gs::Vec2f(scaledOutlineThickness * 2.0f,
						scaledOutlineThickness)
				); 
				
				window::winmain->draw(background); 

				// Handle & render purple interior outline. 

				insideOutline.setSize(size); 
				insideOutline.setPosition(position); 
				insideOutline.setFillColor(gs::Color::Transparent); 
				insideOutline.setOutlineThickness(scaledOutlineThickness); 
				insideOutline.setOutlineColor(gs::Color(100, 0, 220, 180)); 

				window::winmain->draw(insideOutline); 

				// Lastly render the text over top of the background box. 
				gs::draw(window::winmain, text); 
			}

			void updateLoadingScreen() {
				const MenuState currentMenuState = static_cast<MenuState>(
					menuState.getState()
				); 

				if (currentMenuState == MenuState::TitleScreen) {
					titleIntroTransparencyPercentage = std::min(
						titleIntroTransparencyPercentage + 1.0f, 100.0f
					); 

					if (titleIntroTransparencyPercentage >= 100.0f)
						return;
				}

				// Used to scale the size of the progress bar. Note: This value
				// is based on how many assets are loaded. 
				const float progressRatio = 1.0f - (static_cast<float>(
					resourceLoaders.size())
						/ static_cast<float>(highestNumOfResources));

				titleIntroProgressBar.setSize(gs::Vec2f(
					(titleIntroProgressBarOutline.getSize().x
						- (titleIntroProgressBarOutline.getOutlineThickness() 
							* 2.0f)) * progressRatio,
					titleIntroProgressBar.getSize().y)
				); 

				if (resourceLoaders.size() == 0) {
					if (currentMenuState == MenuState::LoadingScreen) {
						menuState.setState(static_cast<int>(MenuState::TitleScreen));
						render::handleAssets();
						handleAssets();
						audio::playMusicTrack(audio::MusicTrack::DoorRemix); 
					}

					tileIntroLoadingText.setString(""); 
				}
				else {
					tileIntroLoadingText.setString("Loading... " 
						+ resourceLoaders.back().fileName);
					tileIntroLoadingText.setPosition(
						(window::defaultWindowWidth / 2.0f) 
							- (tileIntroLoadingText.getGlobalBounds().width / 2.0f), 
						880.0f
					); 
				}
			}
			void updateTitleScreen() {
				titleGameTime.tick(10); 

				titleSinglePlayerButton.update(); 
				titleChangeLogButton.update(); 
				titleCreditsButton.update(); 
				titleOptionsButton.update(); 
				titleQuitGameButton.update(); 
				titleYouTubeButton.update(); 

				if (titleSinglePlayerButton.isSelected
					&& titleSinglePlayerButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::WorldSelection));
					titleIntroTransparencyPercentage = 100.0f; 
					audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
				}
				else if (titleChangeLogButton.isSelected
					&& titleChangeLogButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::ChangeLog)); 
					titleIntroTransparencyPercentage = 100.0f; 
					audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
				}
				else if (titleCreditsButton.isSelected
					&& titleCreditsButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::Credits)); 
					titleIntroTransparencyPercentage = 100.0f; 
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);	
				}
				else if (titleOptionsButton.isSelected
					&& titleOptionsButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::Settings)); 
					titleIntroTransparencyPercentage = 100.0f;
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				}
				else if (titleQuitGameButton.isSelected
					&& titleQuitGameButton.isClickedOn)
				{

					window::winmain->close();
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				}
				else if (titleYouTubeButton.isSelected
					&& titleYouTubeButton.isClickedOn)
				{
					ShellExecute(0, 0, L"https://www.youtube.com/channel/"
						"UCC1p8piMqXKoUGosm8YeGuA", 0, 0, SW_SHOW);
				}

				titleYouTubeUnderline.setFillColor(
					gs::util::approach(
						titleYouTubeButton.getInactiveTextFillColor(),
						titleYouTubeButton.getSelectedTextFillColor(),
						titleYouTubeButton.isSelected ? 100.0f : 0.0f
					)
				);
			}
			void updateWorldSelectionScreen() {
				if (!worldPreviews.empty())
					// Moves slider when the mouse wheel is scrolled. 
					worldSelectionSlider.button.move(
						0.0f, 0.5f * (worldSelectionSlider.getSize().y 
							/ worldPreviews.size()) * -window::mouseDelta
					); 

				worldSelectionPlayButton.update(); 
				worldSelectionEditButton.update(); 
				worldSelectionDeleteButton.update(); 
				worldSelectionCreateButton.update(); 
				worldSelectionCancelButton.update(); 
				worldSelectionSlider.update(); 

				if (worldPreviewSelected != -1) {
					const WorldPreview& worldPreview = 
						*worldPreviews[worldPreviewSelected];

					if (worldSelectionPlayButton.isSelected
						&& worldSelectionPlayButton.isClickedOn) 
					{
						menuState.setState(static_cast<int>(MenuState::InGame));
						window::winmain->setMouseCursorVisible(false);
						audio::playSoundEffect(audio::SoundEffect::ButtonClick);

						world = new World();
						world->loadWorld(worldPreview.folderName);
						audio::initAudioEnviroment(); 
						audio::playMusicTrack(audio::MusicTrack::None); 
					}
					else if (worldSelectionEditButton.isSelected
						&& worldSelectionEditButton.isClickedOn)
					{
						menuState.setState(static_cast<int>(MenuState::WorldEdit)); 
						audio::playSoundEffect(audio::SoundEffect::ButtonClick);

						worldEditNameTextbox.setStoredString(worldPreview.worldName); 
					}
					else if (worldSelectionDeleteButton.isSelected
						&& worldSelectionDeleteButton.isClickedOn)
					{
						audio::playSoundEffect(audio::SoundEffect::ButtonClick);

						std::filesystem::remove_all("saves/" + worldPreview.folderName);
						worldPreviews.erase(worldPreviews.begin() + worldPreviewSelected); 
						worldPreviewSelected = -1; 
					}
				}
				
				if (worldSelectionCreateButton.isSelected
					&& worldSelectionCreateButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::WorldCreate));
					worldPreviewSelected = -1;
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);

					worldCreateWorldNameTextbox.setStoredString("New World"); 
				}
				else if (worldSelectionCancelButton.isSelected
					&& worldSelectionCancelButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::TitleScreen));
					worldPreviewSelected = -1; 
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				}
				
				worldPreviewOffset = 0.0f; 

				// Calculates the offset of the world previews when the world 
				// previews exceed the size of the menu. 
				if (worldPreviews.size() > 4) {
					worldPreviewOffset = (static_cast<float>(worldPreviews.size()) - 4.0f) 
						* worldSelectionSlider.getPercentage(1.0f) * worldPreviewSize.y;
				}

				gs::Hitbox worldPreviewHitbox; 

				worldPreviewHitbox.setSize(worldPreviewSize); 

				// Determine if a world preview is clicked on. 
				for (int worldPreviewIndex = 0; worldPreviewIndex <
					worldPreviews.size(); worldPreviewIndex++)
				{
					worldPreviewHitbox.setPosition(
						getWorldPreviewPosition(worldPreviewIndex) - gs::Vec2f(
							worldPreviewOutline.getOutlineThickness(),
							worldPreviewOutline.getOutlineThickness()
						)
					); 

					if (worldPreviewHitbox.intersects(gs::input::mousePosition)
						&& gs::input::mouseClickL)
					{
						worldPreviewSelected = worldPreviewIndex;
						break;
					}
				}

				// Reset world creation elements. 
				worldCreateWorldNameTextbox.setStoredString("New World"); 
				worldCreateWorldSeedTextbox.setStoredString(""); 
			}
			void updateWorldEditScreen() {
				worldEditNameTextbox.update(); 
				worldEditResetIconButton.update(); 
				worldEditSaveButton.update(); 
				worldEditCancelButton.update(); 

				WorldPreview& worldPreview = 
					*worldPreviews[worldPreviewSelected]; 

				// Can only press the reset icon button when the world being
				// edited has an icon to begin with. 
				if (worldPreview.icon != nullptr
					&& worldEditResetIconButton.isSelected
					&& worldEditResetIconButton.isClickedOn)
				{
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);

					// Reset the preview icon. 
					delete worldPreview.icon; 
					worldPreview.icon = nullptr; 
					// Delete the actual icon from the world folder.
					std::filesystem::remove(
						"saves/" + worldPreview.folderName + "/icon.png"
					);
				}
				else if (worldEditSaveButton.isSelected
					&& worldEditSaveButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::WorldSelection));
					worldPreviewSelected = -1;
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);

					// Temporary world used to write the new properties to. 
					World editedWorld; 

					editedWorld.loadWorld(worldPreview.folderName); 
					editedWorld.name = worldEditNameTextbox.getStoredString(); 
					worldPreview.worldName = editedWorld.name; 
					
					editedWorld.saveWorld(); 
				}
				else if (worldEditCancelButton.isSelected
					&& worldEditCancelButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::WorldSelection));
					worldPreviewSelected = -1;
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				}
			}
			void updateWorldCreateScreen() {
				worldCreateWorldNameTextbox.update();
				worldCreateWorldSeedTextbox.update(); 
				worldCreateNewWorldButton.update();
				worldCreateCancelButton.update();

				if (worldCreateNewWorldButton.isSelected
					&& worldCreateNewWorldButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::InGame));
					window::winmain->setMouseCursorVisible(false);
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);

					std::string worldName = worldCreateWorldNameTextbox.
						getStoredString();
					std::string worldSeed = worldCreateWorldSeedTextbox.
						getStoredString(); 

					if (worldName.empty()) worldName = "New World";

					const std::string basePath = "saves/";

					std::string worldFolderName = worldName;
					std::filesystem::path pathname = basePath + worldName;
					int duplicateIndex = 1;

					// Checks if folder name exists. If it does, increment the
					// duplication index. 
					while (std::filesystem::exists(pathname)) {
						worldFolderName = worldName + " (" + toString(duplicateIndex) + ")";
						pathname = basePath + worldFolderName;
						duplicateIndex++;
					}

					world = new World();
					if (!worldSeed.empty()) world->seed = std::stoi(worldSeed); 
					world->createWorld(worldFolderName, worldName);
					audio::initAudioEnviroment();
					audio::playMusicTrack(audio::MusicTrack::None);
				}
				else if (worldCreateCancelButton.isSelected
					&& worldCreateCancelButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::WorldSelection));
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				}
			}
			void updateChangeLogScreen() {
				changeLogDoneButton.update();

				if (changeLogDoneButton.isSelected
					&& changeLogDoneButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::TitleScreen)); 
					audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
				}
			}
			void updateCreditsScreen() {
				changeLogDoneButton.update();

				if (changeLogDoneButton.isSelected
					&& changeLogDoneButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::TitleScreen));
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				} 
			}
			void updateSettingsScreen() {
				switch (settingsState) {
				case SettingsState::Main:
					settingsDoneButton.update();
					settingsMusicVolumeButton.update();
					settingsMusicVolumeSlider.update();
					settingsSoundVolumeButton.update();
					settingsSoundVolumeSlider.update();
					settingsVideoSettingsButton.update();
					settingsControlsButton.update();

					if (settingsMusicVolumeSlider.isClickedOn) {
						audio::adjustMusicVolume(
							settingsMusicVolumeSlider.getPercentage()
						);
						settingsMusicVolumeButton.setString(
							"Music: " + toString(static_cast<int>(
								audio::musicVolume)) + "%"
						);
					}
					else if (settingsSoundVolumeSlider.isClickedOn) {
						audio::adjustSoundVolume(
							settingsSoundVolumeSlider.getPercentage()
						);
						settingsSoundVolumeButton.setString(
							"Sound: " + toString(static_cast<int>(
								audio::soundVolume)) + "%"
						);
					}
					else if (settingsVideoSettingsButton.isSelected
						&& settingsVideoSettingsButton.isClickedOn)
					{
						settingsState = SettingsState::VideoSettings; 
						audio::playSoundEffect(audio::SoundEffect::ButtonClick);
					}
					else if (settingsControlsButton.isSelected
						&& settingsControlsButton.isClickedOn)
					{
						settingsState = SettingsState::Controls;
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsDoneButton.isSelected
						&& settingsDoneButton.isClickedOn)
					{
						if (static_cast<MenuState>(menuState.getState()) == MenuState::Settings)
							menuState.setState(static_cast<int>(MenuState::TitleScreen));
						else
							pausedSettingsOpen = false;

						audio::playSoundEffect(audio::SoundEffect::ButtonClick);
					}

					break;
				case SettingsState::VideoSettings:
					settingsVideoSettingsFramerateButton.update(); 
					settingsVideoSettingsLightingStyleButton.update(); 
					settingsVideoSettingsRenderBiomeBackgroundButton.update(); 
					settingsVideoSettingsRenderSunAndMoonButton.update(); 
					settingsVideoSettingsRenderParticlesButton.update(); 
					settingsVideoSettingsRenderStarsButton.update(); 
					settingsVideoSettingsGuiScaleButton.update(); 
					settingsVideoSettingsGuiScaleSlider.update(); 
					settingsVideoSettingsDoneButton.update(); 

					if (settingsVideoSettingsFramerateButton.isSelected
						&& settingsVideoSettingsFramerateButton.isClickedOn)
					{
						render::window::renderingFramerate = 
							render::window::renderingFramerate == 30 ? 60 : 30; 
						settingsVideoSettingsFramerateButton.setString(
							"Framerate: " + toString(
								render::window::renderingFramerate
							) + " fps"
						);
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsVideoSettingsLightingStyleButton.isSelected
						&& settingsVideoSettingsLightingStyleButton.isClickedOn)
					{
						lighting::lightingStyle = 
							lighting::lightingStyle == lighting::LightingStyle::Geometry 
								? lighting::LightingStyle::Smooth 
								: lighting::LightingStyle::Geometry; 
						settingsVideoSettingsLightingStyleButton.setString(
							"Lighting Style: " + std::string((lighting::lightingStyle ==
								lighting::LightingStyle::Geometry)
									? "Geometry" : "Smooth")
						); 
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsVideoSettingsRenderBiomeBackgroundButton.isSelected
						&& settingsVideoSettingsRenderBiomeBackgroundButton.isClickedOn)
					{
						shouldBiomeBackgroundBeRendered = !shouldBiomeBackgroundBeRendered; 
						settingsVideoSettingsRenderBiomeBackgroundButton.setString(
							"Render Biome Background: " + std::string(
								shouldBiomeBackgroundBeRendered ? "Yes" : "No")
						);
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsVideoSettingsRenderSunAndMoonButton.isSelected
						&& settingsVideoSettingsRenderSunAndMoonButton.isClickedOn)
					{
						shouldSunAndMoonBeRendered = !shouldSunAndMoonBeRendered; 
						settingsVideoSettingsRenderSunAndMoonButton.setString(
							"Render Sun & Moon: " + std::string(
								shouldSunAndMoonBeRendered ? "Yes" : "No")
						);
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsVideoSettingsRenderParticlesButton.isSelected
						&& settingsVideoSettingsRenderParticlesButton.isClickedOn)
					{
						shouldParticlesBeRendered = !shouldParticlesBeRendered; 
						settingsVideoSettingsRenderParticlesButton.setString(
							"Render Particles: " + std::string(
								shouldParticlesBeRendered ? "Yes" : "No")
						); 
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsVideoSettingsRenderStarsButton.isSelected
						&& settingsVideoSettingsRenderStarsButton.isClickedOn)
					{
						shouldStarsBeRendered = !shouldStarsBeRendered; 
						settingsVideoSettingsRenderStarsButton.setString(
							"Render Stars: " + std::string(
								shouldStarsBeRendered ? "Yes" : "No")
						);
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsVideoSettingsGuiScaleSlider.isClickedOn) {
						guiScale = 1.0f + settingsVideoSettingsGuiScaleSlider.
							getPercentage(9.0f); 
						settingsVideoSettingsGuiScaleButton.setString(
							"Gui Scale: " + toString(guiScale).substr(0,
								toString(guiScale).find('.') + 2)
						);
					}
					else if (settingsVideoSettingsDoneButton.isSelected
						&& settingsVideoSettingsDoneButton.isClickedOn)
					{
						settingsState = SettingsState::Main; 
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}

					break; 
				case SettingsState::Controls:
					// Moves slider when the mouse wheel is scrolled. 
					settingsControlsSlider.button.move(
						0.0f, 0.5f * (settingsControlsSlider.getSize().y
							/ numOfRemappingButtons) * -window::mouseDelta
					);

					settingsControlsSlider.update(); 
					settingsControlsResetKeysButton.update(); 
					settingsControlsDoneButton.update();

					const float remappingButtonsOffset =
						(static_cast<float>(numOfRemappingButtons) - 6.0f)
							* settingsControlsSlider.getPercentage(1.0f) * 100.0f; 
					const bool mouseWithinRange = gs::input::mousePosition.y
						>= 6.0f * tileSize * 2.0f && gs::input::mousePosition.y 
						<= window::defaultWindowHeight - 6.0f * tileSize * 2.5f; 

					bool remappingButtonClicked = false; 

					for (int remappingButtonIndex = 0; remappingButtonIndex <
						numOfRemappingButtons; remappingButtonIndex++)
					{
						RemappingButton& remappingButton = 
							settingsControlsRemappingButtons[remappingButtonIndex]; 

						remappingButton.setPosition(remappingButton.getPosition().x,
							225.0f + (remappingButtonIndex * 100.0f) 
								- remappingButtonsOffset
						); 
						remappingButton.update(); 
						
						if (!mouseWithinRange) {
							remappingButton.isSelected = false; 
							remappingButton.isClickedOn = false; 
						}
						else if (remappingButton.isSelected
							&& remappingButton.isClickedOn)
						{
							remappingButtonSelected = remappingButtonIndex; 
							remappingButtonClicked = true; 
						}

						if (remappingButtonIndex == remappingButtonSelected)
							remappingButton.isSelected = true; 
					}

					if (gs::input::mouseClickL && !remappingButtonClicked)
						remappingButtonSelected = -1; 

					if (remappingButtonSelected != -1) {
						for (sf::Keyboard::Key key = sf::Keyboard::A; key <
							sf::Keyboard::KeyCount; key = static_cast<sf::Keyboard::Key>
								(static_cast<int>(key) + 1))
						{
							if (sf::Keyboard::isKeyPressed(key)) {
								RemappingButton& remappingButton =
									settingsControlsRemappingButtons[remappingButtonSelected]; 

								remappingButton.mappedKey = key; 
								remappingButton.mappedKeyName = 
									input::keyNames[remappingButton.mappedKey]; 

								input::keyMap[remappingButton.key] = key; 
								remappingButtonSelected = -1; 
							}
						}
					}

					if (settingsControlsResetKeysButton.isSelected
						&& settingsControlsResetKeysButton.isClickedOn)
					{
						input::initDefaultKeyBindings(); 
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}
					else if (settingsControlsDoneButton.isSelected
						&& settingsControlsDoneButton.isClickedOn)
					{
						settingsState = SettingsState::Main;
						audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					}

					break; 
				}
			}
			void updateDebugMenu() {
				if (window::ticks % 20 == 0 || fpsString.empty())
					fpsString = toString(window::renderingFramerate) + "/"
						+ toString(window::currentFramerate) + "/" 
						+ toString(window::currentUncappedFramerate);	
			}
			void updatePauseMenu() {
				if (pausedSettingsOpen) {
					updateSettingsScreen(); 
					return; 
				}

				pauseBackToGameButton.update();
				pauseSettingsButton.update(); 
				pauseSaveAndQuitButton.update(); 

				if (pauseBackToGameButton.isSelected
					&& pauseBackToGameButton.isClickedOn) 
				{
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
					gamePaused = false;
				}
				else if (pauseSettingsButton.isSelected
					&& pauseSettingsButton.isClickedOn)
				{
					audio::playSoundEffect(audio::SoundEffect::ButtonClick); 
					pausedSettingsOpen = true; 
				}
				else if (pauseSaveAndQuitButton.isSelected
					&& pauseSaveAndQuitButton.isClickedOn)
				{
					menuState.setState(static_cast<int>(MenuState::TitleScreen));
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);

					world->saveWorld();
					delete world; 
					world = nullptr; 
					deleteEntities();
					createPlayer(); 
					loadWorldPreviews(); 

					gamePaused = false;
					biomeBackgroundSprite.setOrigin(gs::Vec2f()); 
					audio::playMusicTrack(audio::MusicTrack::DoorRemix);
				}
			}
			void updateGameOverMenu() {
				gs::util::approach(&screenShadePercentage, 70.0f, 20.0f);

				gameOverRespawnButton.update(); 
				gameOverTitleMenuButton.update(); 

				if (gameOverRespawnButton.isSelected
					&& gameOverRespawnButton.isClickedOn)
				{
					gameOver = false; 
					*player = Player(); 
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);
				}
				else if (gameOverTitleMenuButton.isSelected
					&& gameOverTitleMenuButton.isClickedOn)
				{
					*player = Player();
					menuState.setState(static_cast<int>(MenuState::TitleScreen));
					audio::playSoundEffect(audio::SoundEffect::ButtonClick);

					world->saveWorld();
					delete world;
					world = nullptr;
					deleteEntities();
					createPlayer();
					loadWorldPreviews();

					gamePaused = false;
					gameOver = false;
					biomeBackgroundSprite.setOrigin(gs::Vec2f());
					audio::playMusicTrack(audio::MusicTrack::DoorRemix);
				}

				inventoryOpen = false; 
				gamePaused = false; 
			}
			void updateCrosshair() {
				shouldCrosshairBeRendered = !(gamePaused || gameOver 
					|| static_cast<MenuState>(menuState.getState()) 
						!= MenuState::InGame); 
				window::winmain->setMouseCursorVisible(!shouldCrosshairBeRendered);
			}
			void updateIngameUI() {
				// Hotbar
				if (!input::isKeyPressed(input::Zoom)) {
					hotbarIndex -= window::mouseDelta;
					hotbarIndex = gs::util::mod(hotbarIndex, 9);
				}

				for (input::Key hotbarKey = input::Hotbar1; hotbarKey <
					input::Hotbar9 + 1; hotbarKey = 
					static_cast<input::Key>(static_cast<int>(hotbarKey) + 1))
				{
					if (input::isKeyClicked(hotbarKey)) {
						hotbarIndex = hotbarKey - input::Hotbar1; 
						break; 
					}
				}

				if (prvsHotbarIndex == hotbarIndex) {
					selectedItemNameTimeRemaining -= 5.0f;
					selectedItemNameTimeRemaining = std::max(
						selectedItemNameTimeRemaining, 0.0f); 
				}
				else {
					selectedItemNameTimeRemaining = window::framerate * 5 * 2; 
					selectedItemName = ItemInfo::itemInfo[
						player->inventory[hotbarIndex].item.id].name; 
				}

				prvsHotbarIndex = hotbarIndex; 
				
				// Debug menu
				if (input::isKeyClicked(input::OpenDebugMenu))
					render::ui::displayDebugMenu = !render::ui::displayDebugMenu;

				if (render::ui::displayDebugMenu)
					render::ui::updateDebugMenu();

				// Inventory 
				
				prvsInventoryOpen = inventoryOpen; 

				if (input::isKeyClicked(input::OpenInventory)
					&& !gamePaused && !gameOver) 
				{
					inventoryMenu = InventoryMenu::Inventory;
					inventoryOpen = !inventoryOpen;
				}
				else if (input::isKeyClicked(input::MenuEscape) && !gameOver) {
					if (!inventoryOpen) {
						gamePaused = !gamePaused;
						pausedSettingsOpen = false; 
						settingsState = SettingsState::Main; 

						if (gamePaused)
							world->saveIcon(); 
					}

					inventoryOpen = false;
				}

				if (!gameOver)
					// Creates cool fading effect into the inventory menu. 
					gs::util::approach(
						&screenShadePercentage, (inventoryOpen || gamePaused)
							? 70.0f : 0.0f, 20.0f
					);

				if (inventoryOpen) {
					itemPreviewString = ""; 
					inventorySlotSelected = -1;
					craftingSlotSelected = -1; 
					craftingOutputSelected = false; 
					furnaceSlotSelected = -1; 
					furnaceOutputSelected = false;
					chestSlotSelected = -1; 

					int itemContainerCount = Player::numOfInventorySlots; 
					std::vector<ItemContainer*> itemContainers; 
					TileEntity* tileEntity = nullptr; 

					for (auto& itemContainer : player->inventory) 
						itemContainers.push_back(&itemContainer);

					// Finds which inventory slot is selected. 
					for (int inventorySlotIndex = 0; inventorySlotIndex <
						Player::numOfInventorySlots; inventorySlotIndex++)
					{
						const gs::Vec2f inventorySlotPosition =
							getInventorySlotPosition(inventorySlotIndex);
						const gs::Vec2f delta = inventorySlotPosition
							- gs::input::mousePosition;

						if (std::abs(delta.x) < (inventorySlotSize.x * guiScale) / 2.0f
							&& std::abs(delta.y) < (inventorySlotSize.y * guiScale) / 2.0f)
						{
							inventorySlotSelected = inventorySlotIndex;
							break;
						}
					}
					
					switch (inventoryMenu) {
					case InventoryMenu::Inventory:
					case InventoryMenu::Crafting:
					{
						const bool fullTable = inventoryMenu == InventoryMenu::Crafting;
						const int craftingSlots = fullTable ? 9 : 4;

						itemContainerCount += craftingSlots;

						// Finds which crafting slot is selected. 
						for (int craftingSlotIndex = 0; craftingSlotIndex <
							craftingSlots; craftingSlotIndex++)
						{
							const gs::Vec2f craftingSlotPosition =
								getCraftingSlotPosition(craftingSlotIndex, fullTable);
							const gs::Vec2f delta = craftingSlotPosition
								- gs::input::mousePosition;

							itemContainers.push_back(&craftingItemGrid[craftingSlotIndex]);

							if (std::abs(delta.x) < (inventorySlotSize.x * guiScale) / 2.0f
								&& std::abs(delta.y) < (inventorySlotSize.y * guiScale) / 2.0f)
							{
								craftingSlotSelected = craftingSlotIndex;
								break;
							}
						}

						// Finds if crafting output slot is selected.

						const gs::Vec2f craftingOutputSlotPosition =
							getCraftingOutputSlotPosition(fullTable);
						const gs::Vec2f delta = craftingOutputSlotPosition
							- gs::input::mousePosition;

						craftingOutputSelected = std::abs(delta.x) <
							(inventorySlotSize.x * guiScale) / 2.0f
							&& std::abs(delta.y) <
							(inventorySlotSize.y * guiScale) / 2.0f;

						outputRecipeContainer = getCraftingGridRecipe(
							craftingItemGrid, fullTable
						);

						// Sets the output container to match recipe if valid. 
						if (outputRecipeContainer.count > 0) {
							const ItemContainer itemContainer(
								Item(outputRecipeContainer.recipe.output.id),
								outputRecipeContainer.count
							);

							craftingOutput = itemContainer;
						}
						// Otherwise, clear output. 
						else
							craftingOutput = ItemContainer(Item());
					}
					case InventoryMenu::Furnace:
					{
						tileEntity = world->getTileEntity(tileEntityPosition);

						if (tileEntity != nullptr)
							itemContainerCount += 2; 

						// Find which furnace slot is selected. 
						for (int furnaceSlot = 0; furnaceSlot < 2;
							furnaceSlot++)
						{
							const gs::Vec2f furnaceSlotPosition =
								getFurnaceInputPosition(furnaceSlot);
							const gs::Vec2f delta = furnaceSlotPosition
								- gs::input::mousePosition;

							if (tileEntity != nullptr)
								itemContainers.push_back(
									&tileEntity->getItemContainer(furnaceSlot)
								);

							if (std::abs(delta.x) < (inventorySlotSize.x * guiScale) / 2.0f
								&& std::abs(delta.y) < (inventorySlotSize.y * guiScale) / 2.0f)
							{
								furnaceSlotSelected = furnaceSlot;
								break;
							}
						}

						// Finding if furnace output slot is selected. 

						const gs::Vec2f furnaceOutputSlotPosition =
							getFurnaceOutputPosition();
						const gs::Vec2f delta = furnaceOutputSlotPosition
							- gs::input::mousePosition;

						furnaceOutputSelected = std::abs(delta.x) <
							(inventorySlotSize.x * guiScale) / 2.0f
							&& std::abs(delta.y) <
							(inventorySlotSize.y * guiScale) / 2.0f;

						}
							break;
					case InventoryMenu::Chest:
					{
						tileEntity = world->getTileEntity(tileEntityPosition);

						if (tileEntity != nullptr)
							itemContainerCount += TileEntity::numOfChestSlots;

						// Find which chest slot is selected. 
						for (int chestSlot = 0; chestSlot < 
							TileEntity::numOfChestSlots; chestSlot++) 
						{
							const gs::Vec2f chestSlotPosition =
								getChestSlotPosition(chestSlot);
							const gs::Vec2f delta = chestSlotPosition
								- gs::input::mousePosition;

							if (tileEntity != nullptr)
								itemContainers.push_back(
									&tileEntity->getItemContainer(chestSlot)
								); 

							if (std::abs(delta.x) < (inventorySlotSize.x * guiScale) / 2.0f
								&& std::abs(delta.y) < (inventorySlotSize.y * guiScale) / 2.0f)
							{
								chestSlotSelected = chestSlot;
								break;
							}
						}
					}
						break; 
					}

					for (auto& itemContainer : itemContainers)
						itemContainer->update(); 

					int slotSelected = Player::numOfInventorySlots;

					if (craftingSlotSelected != -1) slotSelected += craftingSlotSelected;
					else if (furnaceSlotSelected != -1) slotSelected += furnaceSlotSelected;
					else if (chestSlotSelected != -1) slotSelected += chestSlotSelected; 
					else slotSelected = inventorySlotSelected; 

					if (slotSelected != -1) {
						ItemContainer& accessedItemContainer = 
							*itemContainers[slotSelected]; 

						if (accessedItemContainer.count > 0)
							itemPreviewString = accessedItemContainer.item.getName(); 

						const int accessedItemCount = accessedItemContainer.count;
						const int temporaryItemCount = temporaryItemContainer.count; 

						// Inventory events with left click. 
						if (gs::input::mouseClickL) {
							// Adds item to stack. 
							if (Item::compare(accessedItemContainer.item,
								temporaryItemContainer.item)
								&& accessedItemCount > 0
								&& temporaryItemCount > 0)
							{
								temporaryItemContainer = addItemToInventory(
									temporaryItemContainer, &accessedItemContainer, 1
								);

								// If it is unable to add the item because the
								// stack is full, then swap the items instead. 
								if (temporaryItemContainer.count == temporaryItemCount)
									std::swap(temporaryItemContainer,
										accessedItemContainer);
							}
							// Swap items if they don't match types. 
							else {
								std::swap(temporaryItemContainer,
									accessedItemContainer);
							}
						}
						// Inventory events with right click. 
						else if (gs::input::activeMouseClickR) {
							// Divides stack in half. 
							if (gs::input::mouseClickR && temporaryItemCount == 0) {
								temporaryItemContainer = accessedItemContainer;
								temporaryItemContainer.count = std::ceil(
									accessedItemCount / 2.0f
								);

								accessedItemContainer.count = accessedItemCount
									- temporaryItemContainer.count;

								lastInventorySlotSelected = inventorySlotSelected; 
							}
							// Adds single item to stack. 
							else if ((gs::input::mouseClickR 
									|| (gs::input::activeMouseClickR 
									&& lastInventorySlotSelected 
									!= inventorySlotSelected))
								&& (Item::compare(accessedItemContainer.item,
									temporaryItemContainer.item)
									|| accessedItemCount == 0)
								&& temporaryItemCount > 0)
							{
								// Modifies the count of the temporary container
								// by subtracting the number of items returned
								// from the addition from 1. 
								temporaryItemContainer.count -= 1 - addItemToInventory(
									ItemContainer(temporaryItemContainer.item, 1),
									&accessedItemContainer, 1
								).count;

								lastInventorySlotSelected = inventorySlotSelected; 
							}
						}
					}
					else if (craftingOutputSelected) {
						const int recipeItemCount = outputRecipeContainer.recipe.count; 

						int recipeItemsToBeRemoved = 0; 

						itemPreviewString = craftingOutput.item.getName(); 

						if (input::isKeyClicked(input::ItemShift)) {

						}

						if ((gs::input::mouseClickL || gs::input::mouseClickR)
							&& craftingOutput.count > 0) 
						{
							const int itemDifference = recipeItemCount - addItemToInventory(
								ItemContainer(Item(craftingOutput.item.id), recipeItemCount), 
								&temporaryItemContainer, 1
							).count;

							if (itemDifference != recipeItemCount) 
								temporaryItemContainer.count -= itemDifference;
							else
								recipeItemsToBeRemoved = 1; 
						}

						if (recipeItemsToBeRemoved > 0) {
							for (auto& itemContainer : craftingItemGrid) {
								if (itemContainer.count > 0)
									itemContainer.count--; 
							}
						}
					}
					else if (furnaceOutputSelected) {
						if (tileEntity != nullptr) {
							ItemContainer& accessedItemContainer = 
								tileEntity->getItemContainer(2); 

							if (accessedItemContainer.count > 0)
								itemPreviewString = accessedItemContainer.item.getName();

							const int accessedItemCount = accessedItemContainer.count;
							const int temporaryItemCount = temporaryItemContainer.count;
							
							// Adds output items to the mouse.
							if (gs::input::mouseClickL) {
								const int itemDifference = accessedItemCount - addItemToInventory(
									accessedItemContainer, &temporaryItemContainer, 1
								).count;

								accessedItemContainer.count -= itemDifference; 
							}
							// Adds half of output items to the mouse. 
							else if (gs::input::mouseClickR) {
								const int halfItemCount = std::ceil(
									accessedItemContainer.count / 2.0f
								);
								const int itemDifference = halfItemCount - addItemToInventory(
									ItemContainer(accessedItemContainer.item, halfItemCount),
									&temporaryItemContainer, 1
								).count;

								accessedItemContainer.count -= itemDifference;
							}
						}
					}
				}
				else if (prvsInventoryOpen) {
					// Reset recipe output so that it doesn't get displayed in
					// the wrong menu. 
					outputRecipeContainer = RecipeContainer();

					// Puts as many items from the crafting grid into 
					// inventory. Any excess items are simply dropped. 
					for (int craftingSlotIndex = 0; craftingSlotIndex <
						numOfCraftingGridSquares; craftingSlotIndex++)
					{
						ItemContainer& craftingSlotContainer = 
							craftingItemGrid[craftingSlotIndex];

						const ItemContainer remainingItems = addItemToInventory(
							craftingSlotContainer, player->inventory,
							Player::numOfInventorySlots
						); 

						if (remainingItems.count > 0)
							ItemEntity::dropItemEntity(remainingItems, player->position); 

						craftingSlotContainer.count = 0;
					}
				}
				
				if (gamePaused) {
					updatePauseMenu(); 

					if (world == nullptr)
						return; 
				}

				if (gameOver) {
					updateGameOverMenu(); 
				}

				// Handle function keys
				if (input::isKeyClicked(input::HideUI))
					hideUIMenu = !hideUIMenu;
				else if (input::isKeyClicked(input::TakeScreenshot)) {
					const sf::Image& screenshot = takeScreenshot();
					const TimeStruct currentTime = getCurrentTime(); 
					const std::string baseFilename = 
						toString(currentTime.tm_year + 1900) + "-"
						+ toString(currentTime.tm_mon + 1) + "-"
						+ toString(currentTime.tm_mday) + "_"
						+ toString(currentTime.tm_hour) + "."
						+ toString(currentTime.tm_min) + "."
						+ toString(currentTime.tm_sec);

					std::string filename = baseFilename + ".png";
					int duplicationIndex = 1;

					// Checks if filename exists already, if it does, increment
					// the duplication index. 
					while (std::filesystem::exists("screenshots/"
						+ filename))
					{
						filename = baseFilename + " " 
							+ toString(duplicationIndex) + ".png";
						duplicationIndex++; 
					}

					screenshot.saveToFile("screenshots/" + filename); 

					// Handles transition to give the screen a flash. 

					transition.percentage = 255.0f; 
					transition.state = -1; 
				}
				else if (input::isKeyClicked(input::ChangeLightingStyle))
					lighting::lightingStyle = lighting::lightingStyle
						== lighting::LightingStyle::Geometry
							? lighting::LightingStyle::Smooth
							: lighting::LightingStyle::Geometry;
				else if (input::isKeyClicked(input::EnableFullBright))
					lighting::fullBrightEnabled = !lighting::fullBrightEnabled;
				else if (input::isKeyClicked(input::ChangeFramerate))
					window::renderingFramerate = (window::renderingFramerate
						== 30) ? 60 : 30;
				else if (input::isKeyClicked(input::EnterSpectatorMode))
					gameMode = gameMode != GameMode::Spectator
						? GameMode::Spectator : GameMode::Survival;
				else if (input::isKeyPressed(input::IncrementDayTime))
					world->gameTime.tick(20); 

				// Temperary debug tile placement system. 
				if (!inventoryOpen && gameMode != GameMode::Survival) {
					if (gs::input::activeMouseClickR) {
						if (useDebugBlock)
							world->placeBlock(
								mouseTilePosition, Block(debugBlockId), 
								World::PlaceFilter::Replace, true
							);
						else
							world->placeWall(
								mouseTilePosition, Wall(debugWallId),
								World::PlaceFilter::Replace, true
							);
					}
					else if (gs::input::activeMouseClickL) {
						if (useDebugBlock)
							world->breakBlock(mouseTilePosition, false, true);
						else
							world->breakWall(mouseTilePosition, false, true); 
					}
					if (gs::input::mouseClickM)
						useDebugBlock = !useDebugBlock;

					if (window::mouseDelta != 0) {
						if (useDebugBlock) {
							int newBlockId = static_cast<int>(debugBlockId);
							newBlockId += window::mouseDelta;

							gs::util::clamp(&newBlockId, 1, static_cast<int>(Block::End) - 1);

							debugBlockId = static_cast<Block::Id>(newBlockId);
						}
						else {
							int newWallId = static_cast<int>(debugWallId);
							newWallId += window::mouseDelta;

							gs::util::clamp(&newWallId, 1, static_cast<int>(Wall::End) - 1);

							debugWallId = static_cast<Wall::Id>(newWallId);
						}
					}
				}
			}

			void renderLoadingScreen() {
				// No longer renders loading screen once fully transparent. 
				if (titleIntroTransparencyPercentage >= 100.0f)
					return; 

				// Color of everything on the loading screen. Note: This 
				// becomes more transparent as the title screen is loaded. 
				const gs::Color loadingColor = gs::util::approach(
					gs::Color::White, gs::Color::Transparent,
					titleIntroTransparencyPercentage
				);

				titleIntroLogoSprite.setColor(loadingColor); 
				titleIntroProgressBar.setFillColor(loadingColor); 
				titleIntroProgressBarOutline.setOutlineColor(loadingColor); 
				tileIntroLoadingText.setFillColor(loadingColor); 

				window::winmain->draw(titleIntroLogoSprite); 
				window::winmain->draw(titleIntroProgressBar);
				window::winmain->draw(titleIntroProgressBarOutline); 
				window::winmain->draw(tileIntroLoadingText); 
			}
			void renderTitleScreen() {
				// Handle & render background.

				biomeBackgroundSprite.setTexture(
					biomeBackgroundTextures[titleBackgroundIndex]
				);
				biomeBackgroundSprite.setScale(1.0f, 1.0f);

				const gs::Color biomeBackgroundColor = gs::util::approach(
					lighting::sunlightColor, gs::util::approach(
						lighting::moonlightColor, gs::Color::White, 10.0f),
					lighting::sunlightBrightness
				);

				biomeBackgroundSprite.setColor(biomeBackgroundColor);

				// Renders 2 backgrounds to maintain a seemless transition. 
				for (int backgroundOffset = 0; backgroundOffset < 2; 
					backgroundOffset++) 
				{
					// Background horizontal shift, based on ticks. 
					const float xoffset = -std::fmod(
						static_cast<float>(window::ticks),
						window::defaultWindowWidth
					); 

					biomeBackgroundSprite.setPosition(
						xoffset + (backgroundOffset * window::defaultWindowWidth),
						0.0f
					); 

					window::winmain->draw(biomeBackgroundSprite);
				}

				// Render title blur. 
				window::winmain->draw(titleBlurSprite); 

				// Render title logo. 
				window::winmain->draw(titleLogoSprite); 

				// Render text. 

				text.setShadow(true);
				text.setShadowOffset(0.0f, 4.0f);

				renderText(
					"A fan game created by CodeNoodles",
					gs::Vec2f(10.0f, 1050.0f), 0, false, gs::Vec2f(1.0f, 1.0f),
					gs::Color::White
				);

				// Handle & render splash-text. 

				const std::string& splashTextString = titleSplashTexts[titleSplashIndex]; 

				float splashTextScale = std::sin(
					static_cast<float>(window::ticks) / 12.0f
				); 
				splashTextScale = 0.15f * std::pow(splashTextScale, 2.0f);
				splashTextScale += 2.5f; 
				splashTextScale *= std::pow(
					1.02f, -static_cast<int>(splashTextString.size())
				); 

				titleSplashText.setPosition(1400.0f, 220.0f); 
				titleSplashText.setString(splashTextString); 
				titleSplashText.getText().setOrigin(
					titleSplashText.getText().getLocalBounds().width / 2.0f,
					titleSplashText.getText().getLocalBounds().height / 2.0f
				);
				// Rotates text to give it a slanted look. 
				titleSplashText.getText().setRotation(-15.0f); 
				titleSplashText.setScale(splashTextScale, splashTextScale); 

				gs::draw(window::winmain, titleSplashText);

				// Render UI components. 

				renderMenuButton(titleSinglePlayerButton); 
				renderMenuButton(titleChangeLogButton); 
				renderMenuButton(titleCreditsButton); 
				renderMenuButton(titleOptionsButton); 
				renderMenuButton(titleQuitGameButton);
				gs::draw(window::winmain, titleYouTubeButton);
				window::winmain->draw(titleYouTubeUnderline); 
				 
				text.setShadow(false);
			}
			void renderMenuBackground(bool multiLayer, int layer) {
				auto generateBackgroundTextureBounds = [](float height)
					-> sf::IntRect
				{
					sf::IntRect bounds;

					bounds.left = 0;
					bounds.top = 0;
					bounds.width = window::defaultWindowWidth
						/ optionsBackgroundSprite.getScale().x;
					bounds.height = height;

					return bounds;
				};
				const gs::Color backgroundColor = gs::Color(30, 30, 30);
				const gs::Color foregroundColor = gs::Color(60, 60, 60);

				if (layer < 1) {
					optionsBackgroundSprite.setColor(
						multiLayer ? backgroundColor : foregroundColor
					);
					optionsBackgroundSprite.setPosition(0.0f, 0.0f);
					optionsBackgroundSprite.setTextureRect(
						generateBackgroundTextureBounds(window::defaultWindowHeight
							/ optionsBackgroundSprite.getScale().y)
					);

					window::winmain->draw(optionsBackgroundSprite);
				}
				if (multiLayer && (layer == -1 || layer == 1)) {
					optionsBackgroundSprite.setColor(foregroundColor);

					const float backgroundTextureHeight =
						optionsBackgroundTexture.getSize().y;

					optionsBackgroundSprite.setTextureRect(
						generateBackgroundTextureBounds(backgroundTextureHeight * 2)
					);

					window::winmain->draw(optionsBackgroundSprite);

					const float bottomTextureHeight = backgroundTextureHeight * 2.5;

					optionsBackgroundSprite.setPosition(0.0f,
						window::defaultWindowHeight - (bottomTextureHeight
							* optionsBackgroundSprite.getScale().y)
					);
					optionsBackgroundSprite.setTextureRect(
						generateBackgroundTextureBounds(bottomTextureHeight)
					);

					window::winmain->draw(optionsBackgroundSprite);
				}
			}
			void renderWorldPreview(const WorldPreview& worldPreview, int index) {
				const gs::Vec2f position = getWorldPreviewPosition(index); 

				// Renders missing world icon if no icon was loaded. 
				if (worldPreview.icon == nullptr) {
					missingWorldSprite.setPosition(position);

					window::winmain->draw(missingWorldSprite);
				}
				else {
					sf::Sprite iconSprite(*worldPreview.icon); 
					iconSprite.setPosition(position); 

					window::winmain->draw(iconSprite); 
				}

				const gs::Color subTextColor = gs::Color(127, 127, 127);
				const float subTextOffset = 42.0f; 

				renderText(
					worldPreview.worldName, position 
						+ gs::Vec2f(150.0f, 5.0f), 0, false, 
					gs::Vec2f(1.5f, 1.5f)
				);
				renderText(
					worldPreview.folderName, position
						+ gs::Vec2f(150.0f, 8.0f + subTextOffset), 0, false,
					gs::Vec2f(1.5f, 1.5f), subTextColor
				);

				const std::string versionString = worldPreview.versionNumber 
					!= -1.0f ? ", Version: " + toString(
						worldPreview.versionNumber).substr(0, 3) : "";

				renderText(
					"Survival Mode" + versionString, position 
						+ gs::Vec2f(150.0f, 8.0f + subTextOffset * 2.0f), 0, 
					false, gs::Vec2f(1.5f, 1.5f), subTextColor
				);

				if (worldPreviewSelected == index) {
					worldPreviewOutline.setPosition(position - gs::Vec2f(
						worldPreviewOutline.getOutlineThickness(),
						worldPreviewOutline.getOutlineThickness()
					)); 

					window::winmain->draw(worldPreviewOutline); 
				}
			}
			void renderWorldSelectionScreen() {
				renderMenuBackground(true, 0); 

				text.setShadow(true); 
				text.setShadowOffset(0.0f, 4.0f); 

				for (int worldPreviewIndex = 0; worldPreviewIndex <
					worldPreviews.size(); worldPreviewIndex++) 
				{
					const WorldPreview& worldPreview = 
						*worldPreviews[worldPreviewIndex]; 

					renderWorldPreview(worldPreview, worldPreviewIndex);
				}

				renderMenuBackground(true, 1);

				renderText(
					"Select World", gs::Vec2f(window::defaultWindowWidth 
						/ 2.0f, 100.0f), 0, true, gs::Vec2f(1.5f, 1.5f), 
					gs::Color::White 
				); 

				// Handle & render UI components.

				const bool worldSelected = worldPreviewSelected >= 0; 
				const float sliderButtonSizeScaler = 1.0f / (std::log(std::max(
					0.0f, worldPreviews.size() - 4.0f) + 1.0f) + 1.0f);

				worldSelectionSlider.button.setSize(
					worldSelectionSlider.button.getSize().x, 
					worldSelectionSlider.getSize().y * sliderButtonSizeScaler
				); 

				renderMenuButton(worldSelectionPlayButton, !worldSelected); 
				renderMenuButton(worldSelectionEditButton, !worldSelected); 
				renderMenuButton(worldSelectionDeleteButton, !worldSelected); 
				renderMenuButton(worldSelectionCreateButton); 
				renderMenuButton(worldSelectionCancelButton); 
				gs::draw(window::winmain, worldSelectionSlider); 

				text.setShadow(false); 
			}
			void renderWorldEditScreen() {
				renderMenuBackground(false);

				text.setShadow(true);
				text.setShadowOffset(0.0f, 4.0f);

				renderText(
					"Edit World", gs::Vec2f(window::defaultWindowWidth
						/ 2.0f, 80.0f), 0, true, gs::Vec2f(1.5f, 1.5f),
					gs::Color::White
				);
				renderText(
					"World Name", gs::Vec2f(565.0f, 125.0f), 0, false, 
					gs::Vec2f(1.5f, 1.5f), gs::Color(127, 127, 127)
				);

				// Handle & render UI components. 

				WorldPreview& worldPreview =
					*worldPreviews[worldPreviewSelected];

				worldEditNameTextbox.setOutlineColor(
					worldEditNameTextbox.getActive() ? gs::Color::White 
						: gs::Color(127, 127, 127)
				);

				gs::draw(window::winmain, worldEditNameTextbox); 
				renderMenuButton(
					worldEditResetIconButton, worldPreview.icon == nullptr
				); 
				renderMenuButton(worldEditSaveButton); 
				renderMenuButton(worldEditCancelButton); 

				text.setShadow(false);
			}
			void renderWorldCreateScreen() {
				renderMenuBackground(false);

				text.setShadow(true);
				text.setShadowOffset(0.0f, 4.0f);

				renderText(
					"World Creation", gs::Vec2f(window::defaultWindowWidth
						/ 2.0f, 80.0f), 0, true, gs::Vec2f(1.5f, 1.5f),
					gs::Color::White
				);
				renderText(
					"World Name", gs::Vec2f(565.0f, 125.0f), 0, false,
					gs::Vec2f(1.5f, 1.5f), gs::Color(127, 127, 127)
				);
				renderText(
					"Seed", gs::Vec2f(565.0f, 285.0f), 0, false,
					gs::Vec2f(1.5f, 1.5f), gs::Color(127, 127, 127)
				);

				// Handle & render UI components. 

				gs::draw(window::winmain, worldCreateWorldNameTextbox); 
				gs::draw(window::winmain, worldCreateWorldSeedTextbox); 
				renderMenuButton(worldCreateNewWorldButton); 
				renderMenuButton(worldCreateCancelButton); 

				text.setShadow(false);
			}
			void renderChangeLogScreen() {
				renderMenuBackground(true, 0);

				renderText(
					"          Version 1.0          ",
					gs::Vec2f(window::defaultWindowWidth / 2.0f, 247.0f), 0, true,
					gs::Vec2f(1.5f, 1.5f), gs::Color::Black, 0.0f, gs::Color(), 
					8.0f, gs::Color(0, 255, 0, 127)
				);
				renderText(
					"Inital release. Note: MANY BUGS CURRENTLY EXIST",
					gs::Vec2f(window::defaultWindowWidth / 2.0f, 310.0f), 0, true,
					gs::Vec2f(1.0f, 1.0f)
				);
				renderText(
					"- Includes 9 biomes"
					"\n- Includes 5 mobs (3 hostile, 2 passive)"
					"\n- Includes basic survival mode", 
					gs::Vec2f(window::defaultWindowWidth / 2.0f, 400.0f), 0, true,
					gs::Vec2f(1.0f, 1.0f), gs::Color(200, 200, 200)
				);

				renderMenuBackground(true, 1);

				renderText(
					"Welecome to the change-log page! This will offer a"
					"\ndescription of every major update to this game.",
					gs::Vec2f(window::defaultWindowWidth / 2.0f, 100.0f), 0, true,
					gs::Vec2f(1.2f, 1.2f), gs::Color(200, 200, 200)
				);
				renderMenuButton(changeLogDoneButton);
			}
			void renderCreditsScreen() {
				renderMenuBackground(false);

				renderText(
					"COMING SOON",
					gs::Vec2f(window::defaultWindowWidth / 2.0f, 400.0f), 0, true,
					gs::Vec2f(2.0f, 2.0f)
				);

				renderMenuButton(changeLogDoneButton);
			}
			void renderSettingsScreen() {
				const bool useMultiLayerMenu = settingsState == SettingsState::Controls;

				renderMenuBackground(useMultiLayerMenu, useMultiLayerMenu ? 0 : -1); 

				switch (settingsState) {
				case SettingsState::Main:
					renderText(
						"Options", gs::Vec2f(window::defaultWindowWidth
							/ 2.0f, 100.0f), 0, true, gs::Vec2f(1.5f, 1.5f),
						gs::Color::White
					);

					// Handle & render UI components.

					renderMenuButton(settingsDoneButton);
					renderMenuSlider(settingsMusicVolumeButton,
						settingsMusicVolumeSlider);
					renderMenuSlider(settingsSoundVolumeButton,
						settingsSoundVolumeSlider);
					renderMenuButton(settingsVideoSettingsButton);
					renderMenuButton(settingsControlsButton);
					break;
				case SettingsState::VideoSettings:
					renderText(
						"Video Settings", gs::Vec2f(window::defaultWindowWidth
							/ 2.0f, 100.0f), 0, true, gs::Vec2f(1.5f, 1.5f),
						gs::Color::White
					);

					// Handle & render UI components.

					renderMenuButton(settingsVideoSettingsFramerateButton); 
					renderMenuButton(settingsVideoSettingsLightingStyleButton); 
					renderMenuButton(settingsVideoSettingsRenderBiomeBackgroundButton); 
					renderMenuButton(settingsVideoSettingsRenderSunAndMoonButton); 
					renderMenuButton(settingsVideoSettingsRenderParticlesButton); 
					renderMenuButton(settingsVideoSettingsRenderStarsButton); 
					renderMenuSlider(settingsVideoSettingsGuiScaleButton,
						settingsVideoSettingsGuiScaleSlider); 
					renderMenuButton(settingsVideoSettingsDoneButton); 
					break; 
				case SettingsState::Controls:
					const float sliderButtonSizeScaler = 1.0f / (std::log(std::max(
						0.0f, numOfRemappingButtons - 6.0f) + 1.0f) + 1.0f);

					settingsControlsSlider.button.setSize(
						settingsControlsSlider.button.getSize().x,
						settingsControlsSlider.getSize().y * sliderButtonSizeScaler
					);

					gs::draw(window::winmain, settingsControlsSlider); 

					for (int remappingButtonIndex = 0; remappingButtonIndex <
						numOfRemappingButtons; remappingButtonIndex++)
					{
						RemappingButton& remappingButton =
							settingsControlsRemappingButtons[remappingButtonIndex];

						renderControlRemappingButton(
							remappingButton, remappingButtonIndex 
								== remappingButtonSelected
						); 
					}

					if (useMultiLayerMenu)
						renderMenuBackground(useMultiLayerMenu, 1);

					renderText(
						"Controls", gs::Vec2f(window::defaultWindowWidth
							/ 2.0f, 100.0f), 0, true, gs::Vec2f(1.5f, 1.5f),
						gs::Color::White
					);

					renderMenuButton(settingsControlsResetKeysButton); 
					renderMenuButton(settingsControlsDoneButton); 
					break; 
				}
			}
			void renderDebugMenu() {
				auto truncateFloat = [](float value, int decimalPlaces = 2)
					-> std::string 
				{
					std::string valueString = toString(value); 
					size_t decimalSpot = valueString.find("."); 

					// Shortens string if decimal is located.  
					if (decimalSpot != std::string::npos)
						valueString = valueString.substr(
							0, decimalSpot + decimalPlaces + 1
						); 

					return valueString; 
				};

				const float backgroundThickness = 8.0f; 

				auto printSpace = [&]() {
					renderText(
						"", gs::Vec2f(15.0f, prvsTextBounds.top
							+ prvsTextBounds.height + (2.0f * backgroundThickness))
					);
				}; 

				const Biome::Id currentBiome = getBiome(
					World::getChunkOffset(player->position.x)
				); 

				prepareText(
					0, gs::Vec2f(1.5f, 1.5f), gs::Color::White, 0.0f,
					gs::Color::Black, backgroundThickness, 
					gs::Color(60, 60, 60, 150), false
				); 
				renderText(window::gameTitle, gs::Vec2f(15.0f, 15.0f));
				renderText(
					fpsString + " fps", gs::Vec2f(15.0f, prvsTextBounds.top 
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				printSpace(); 

				renderText(
					toString(tilesRendered) + " tiles rendered",
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				renderText(
					toString(lighting::lightsRendered) + " lights rendered", 
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				); 
				renderText(
					"Blocks updated: " + toString(world->getNumOfBlocksUpdated()),
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				renderText(
					"Light update rate: " + toString(lighting::lightUpdateRate),
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				renderText(
					"Entities loaded: " + toString(Entity::numOfEntities) + "/h"
						+ toString(Mob::numOfHostileMobs) + "/p"
						+ toString(Mob::numOfPassiveMobs),
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				printSpace(); 

				renderText(
					"XY: " + truncateFloat(player->position.x) + "/" 
						+ truncateFloat(player->position.y),
					gs::Vec2f(15.0f, prvsTextBounds.top + prvsTextBounds.height 
						+ (2.0f * backgroundThickness))
				);
				renderText(
					"Biome: " + Biome::biomeStrings[currentBiome],
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				printSpace(); 

				renderText(
					"Seed: " + toString(world->seed),
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
				renderText(
					"Game Time: " + toString(world->gameTime.gameTicks),
					gs::Vec2f(15.0f, prvsTextBounds.top
						+ prvsTextBounds.height + (2.0f * backgroundThickness))
				);
			}
			void renderPauseMenu() {
				if (pausedSettingsOpen) {
					renderSettingsScreen(); 
					return; 
				}

				text.setShadow(true);
				text.setShadowOffset(0.0f, 4.0f);

				renderText(
					"Pause Menu", gs::Vec2f(window::defaultWindowWidth / 2.0f, 
						300.0f), 0, true, gs::Vec2f(1.5f, 1.5f),
					gs::Color::White
				);

				// Handle & render UI components. 

				renderMenuButton(pauseBackToGameButton); 
				renderMenuButton(pauseSettingsButton); 
				renderMenuButton(pauseSaveAndQuitButton); 

				text.setShadow(false); 
			}
			void renderGameOverMenu() {
				text.setShadow(true);
				text.setShadowOffset(8.0f, 8.0f);

				renderText(
					"Game over!", gs::Vec2f(window::defaultWindowWidth
						/ 2.0f, 300.0f), 0, true, gs::Vec2f(2.5f, 2.5f),
					gs::Color::White
				);

				// Handle & render UI components. 

				renderMenuButton(gameOverRespawnButton); 
				renderMenuButton(gameOverTitleMenuButton); 

				text.setShadow(false);
			}
			void renderCrosshair() {
				if (!shouldCrosshairBeRendered)
					return;

				cursorSprite.setPosition(gs::input::mousePosition); 

				window::winmain->draw(
					cursorSprite, sf::BlendMode(sf::BlendMode::OneMinusDstColor, 
						sf::BlendMode::OneMinusSrcAlpha)
				);
			}
			void renderTileSelectionOutline() {
				const Block blockSelected = world->getBlock(mouseTilePosition); 
				const Wall wallSelected = world->getWall(mouseTilePosition); 
				const bool renderTileSelectionOutline = blockInteraction
					? !blockSelected.isEmpty() && !blockSelected.isFluid()
					: !wallSelected.isEmpty() && (blockSelected.isEmpty() 
						|| blockSelected.getVar(BlockInfo::hasTransparency));

				if (renderTileSelectionOutline) {
					tileSelectionOutline.setSize(scalePosition({ 1.0f, 1.0f }));
					tileSelectionOutline.setPosition(transformPosition(
						gs::Vec2f(mouseTilePosition.x, mouseTilePosition.y)
					));

					worldRenderTexture.draw(tileSelectionOutline);
				}
			}
			void renderIngameUI() {
				if (gameMode != GameMode::Survival) {
					if (useDebugBlock)
						renderBlockPreview(debugBlockId, mouseTilePosition);
					else
						renderWallPreview(debugWallId, mouseTilePosition);
				}

				if (!hideUIMenu) {
					// Tile preview

					const ItemContainer itemContainerSelected = 
						player->inventory[hotbarIndex]; 

					if (itemContainerSelected.count > 0) {
						const ItemInfo::PlacementType itemSelectedType = 
							static_cast<ItemInfo::PlacementType>(
								itemContainerSelected.item.getVar(ItemInfo::type));

						// Handle block preview. 
						if (itemSelectedType == ItemInfo::PlacementType::Block) {
							const Block::Id blockId = static_cast<Block::Id>(
								itemContainerSelected.item.getVar(ItemInfo::tileIndex)
							);

							if (world->isValidBlockPlacementLocation(
									mouseTilePosition, blockId)) 
								renderBlockPreview(blockId, mouseTilePosition);
						}
						// Handle wall preview. 
						else if (itemSelectedType == ItemInfo::PlacementType::Wall) {
							const Wall::Id wallId = static_cast<Wall::Id>(
								itemContainerSelected.item.getVar(ItemInfo::tileIndex)
							);
							 
							if (world->isValidWallPlacementLocation(
									mouseTilePosition, wallId))
								renderWallPreview(wallId, mouseTilePosition); 
						}
					}

					// Hotbar
					hotbarSprite.setScale(guiScale, guiScale);
					hotbarSprite.setOrigin(
						hotbarTexture.getSize().x / 2.0f,
						hotbarTexture.getSize().y / 2.0f
					);
					hotbarSprite.setPosition(
						window::defaultWindowWidth / 2.0f,
						window::defaultWindowHeight
					);
					hotbarSprite.move(
						0.0f, -((hotbarTexture.getSize().y / 2.0f) * guiScale)
					);

					window::winmain->draw(hotbarSprite);

					float hotbarStartXpos = hotbarSprite.getPosition().x;
					hotbarStartXpos -= (hotbarTexture.getSize().x / 2.0f) * guiScale;
					hotbarStartXpos += (hotbarTexture.getSize().y / 2.0f) * guiScale;

					const float hotbarGapWidth = 20.0f * guiScale;

					hotbarSelectorSprite.setScale(guiScale, guiScale);
					hotbarSelectorSprite.setPosition(
						hotbarStartXpos + (hotbarGapWidth * hotbarIndex),
						hotbarSprite.getPosition().y
					);

					for (int hotbarSlot = 0; hotbarSlot < 
						Player::numOfHotbarSlots; hotbarSlot++) 
					{
						const ItemContainer& itemContainer = 
							player->inventory[hotbarSlot];

						renderInventoryItem(
							itemContainer, 
							gs::Vec2f(hotbarStartXpos + (hotbarSlot * hotbarGapWidth),
								hotbarSprite.getPosition().y
						)); 
					}

					window::winmain->draw(hotbarSelectorSprite);

					// Xp-bar 
					xpBarSprite.setTexture(xpBarTextures[0]); 
					xpBarSprite.setScale(guiScale, guiScale); 
					xpBarSprite.setPosition(hotbarSprite.getPosition()); 
					xpBarSprite.move(
						0.0f, -(hotbarTexture.getSize().y / 2.0f) * guiScale 
							- (2.0f * guiScale)
					); 

					window::winmain->draw(xpBarSprite); 

					// Hearts

					const float heartWidth = (heartSprite.getTexture()->getSize().x
						* guiScale) - guiScale; 
					const gs::Vec2f heartStartPosition = gs::Vec2f(
						xpBarSprite.getPosition().x 
							- ((xpBarSprite.getTexture()->getSize().x / 2.0f)
							* guiScale) + (heartWidth / 2.0f) + guiScale,
						xpBarSprite.getPosition().y
							- (xpBarSprite.getTexture()->getSize().y * guiScale)
							- guiScale
					);

					heartSprite.setScale(guiScale, guiScale); 

					for (int heartIndex = 0; heartIndex < 10; heartIndex++) {
						const int healthDifference = player->health
							- (heartIndex * 2.0f); 
						const int textureIndex = healthDifference > 0.0f 
							? (healthDifference > 1 ? 0 : 1) : 2; 

						heartSprite.setTexture(heartTextures[textureIndex]); 
						heartSprite.setPosition(heartStartPosition); 
						heartSprite.move(heartWidth * heartIndex, 0.0f);

						// Animates hearts when at or below 3 full hearts. 
						if (player->health <= 6) {
							const float healthLost = 6 - player->health; 

							float heartOffset = std::sin(heartIndex
								+ (static_cast<float>(window::ticks) 
									/ (0.25f + std::max(1.0f, 6 - healthLost)))); 
							heartOffset *= (healthLost + 1) / 6.0f; 

							heartSprite.move(0.0f, heartOffset * guiScale);
						}

						window::winmain->draw(heartSprite); 
					}

					// Hunger 

					const float hungerWidth = (hungerSprite.getTexture()->getSize().x
						* guiScale) - guiScale; 
					const gs::Vec2f hungerStartPosition = gs::Vec2f(
						xpBarSprite.getPosition().x
							+ ((xpBarSprite.getTexture()->getSize().x / 2.0f)
							* guiScale) - (heartWidth / 2.0f) - guiScale,
						heartStartPosition.y
					);

					hungerSprite.setScale(guiScale, guiScale);

					for (int hungerIndex = 0; hungerIndex < 10; hungerIndex++) {
						int textureIndex = 2;

						const int healthDifference = player->foodLevel 
							- (hungerIndex * 2);

						if (healthDifference > 0)
							textureIndex = healthDifference > 1 ? 0 : 1;

						hungerSprite.setTexture(hungerTextures[textureIndex]);
						hungerSprite.setPosition(hungerStartPosition);
						hungerSprite.move(-heartWidth * hungerIndex, 0.0f);

						window::winmain->draw(hungerSprite);
					}

					// Switcher
					switcherSprite.setTexture(switcherTextures[blockInteraction ? 0 : 1]);
					switcherSprite.setScale(gs::Vec2f(guiScale, guiScale) * 0.75f); 
					switcherSprite.setOrigin(gs::Vec2f(
						switcherSprite.getTexture()->getSize()) * 0.5f
					); 
					switcherSprite.setPosition(
						hotbarSprite.getPosition() - gs::Vec2f(
							hotbarSprite.getTexture()->getSize().x, 0.0f
						) * guiScale * 0.5f
					); 
					switcherSprite.move(-16.0f * guiScale, 0.0f); 
					  
					window::winmain->draw(switcherSprite); 

					// Item name preview 
					if (selectedItemNameTimeRemaining > 0.0f) {
						const gs::Vec2f itemNamePosition = gs::Vec2f(
							window::defaultWindowWidth / 2.0f,
							window::defaultWindowHeight - (46.0f * guiScale)
						); 
						const gs::Vec2f itemNameScale = gs::Vec2f(
							guiScale * 0.3f, guiScale * 0.3f); 
						const gs::Color itemNameColor = gs::Color(
							255, 255, 255, gs::util::approach(0.0f, 255.0f,
								std::min(selectedItemNameTimeRemaining, 100.0f)
							)
						); 

						renderText(
							selectedItemName, itemNamePosition, 0, true, 
							itemNameScale, itemNameColor
						);
					}

					// Debug menu
					if (displayDebugMenu)
						renderDebugMenu();

					if (screenShadePercentage > 0.0f) {
						screenShade.setFillColor(gs::util::approach(
							gs::Color::White, gs::Color::Black, screenShadePercentage
						));

						window::winmain->draw(screenShade, sf::BlendMultiply); 
					}

					// Inventory
					if (inventoryOpen) {
						// Handle and render the container sprite. 

						containerSprite.setTexture(
							containerTextures[static_cast<int>(inventoryMenu)]
						);
						containerSprite.setScale(guiScale, guiScale);
						containerSprite.setOrigin(
							containerSprite.getTexture()->getSize().x / 2.0f,
							containerSprite.getTexture()->getSize().y / 2.0f
						);
						containerSprite.setPosition(
							window::defaultWindowWidth / 2.0f,
							window::defaultWindowHeight / 2.0f
						);

						window::winmain->draw(containerSprite);

						// Handle inventory slot shade. 

						inventorySlotShadePosition = gs::Vec2f(); 
						ItemContainer itemContainerSelected;  

						if (inventorySlotSelected != -1) {
							inventorySlotShadePosition = 
								getInventorySlotPosition(inventorySlotSelected);

							itemContainerSelected = player->inventory[inventorySlotSelected]; 
						}

						const bool fullTable = inventoryMenu == InventoryMenu::Crafting;

						switch (inventoryMenu) {
						case InventoryMenu::Inventory:
						{
							const float originalCameraScale = cameraScale; 
							const float eyeline = 26.0f;

							Player previewPlayer = *player; 
							Player::numOfEntities += 2; 

							cameraScale = guiScale * 1.5f;

							const float positionDivisor = scaleValue(1.0f); 

							previewPlayer.position = cameraPosition 
								/ positionDivisor; 
							previewPlayer.position += gs::Vec2f(
								window::defaultWindowWidth / 2.0f, 
								window::defaultWindowHeight / 2.0f
							) / positionDivisor; 
							previewPlayer.position += gs::Vec2f(-38.0f, -38.0f) 
								/ (positionDivisor / guiScale); 

							float& headAngle = previewPlayer.modelTransform.
								transforms[ModelTransform::Head].angle;

							headAngle = gs::util::angleBetween(
								gs::input::mousePosition,
								render::transformPosition(previewPlayer.position 
									+ gs::Vec2f(0.0f, (previewPlayer.size.y / 2.0f) 
										- (eyeline / 16.0f)))
							);
							headAngle -= 180.0f;

							previewPlayer.modelTransform.calculateDirection();

							renderPlayerEntity(previewPlayer, *window::winmain); 

							cameraScale = originalCameraScale; 
						}
						case InventoryMenu::Crafting: 	
							if (craftingSlotSelected != -1) {
								inventorySlotShadePosition = getCraftingSlotPosition(
									craftingSlotSelected, fullTable);
							}
							else if (craftingOutputSelected) {
								inventorySlotShadePosition =
									getCraftingOutputSlotPosition(fullTable);
							}

							break; 
						case InventoryMenu::Furnace:
							if (furnaceSlotSelected != -1) {
								inventorySlotShadePosition = getFurnaceInputPosition(
									furnaceSlotSelected); 
							}
							else if (furnaceOutputSelected) {
								inventorySlotShadePosition = 
									getFurnaceOutputPosition(); 
							}

							break; 
						case InventoryMenu::Chest:
							if (chestSlotSelected != -1) {
								inventorySlotShadePosition = getChestSlotPosition(
									chestSlotSelected); 
							}
							break; 
						}

						// Render inventory slot shade if required. 
						if (inventorySlotShadePosition != gs::Vec2f()) {
							inventorySlotShade.setScale(guiScale, guiScale); 
							inventorySlotShade.setPosition(inventorySlotShadePosition); 

							window::winmain->draw(inventorySlotShade); 
						}

						gs::Vec2f containerSpriteCorner = containerSprite.getPosition();

						containerSpriteCorner.x -= (containerSprite.
							getTexture()->getSize().x / 2.0f) * guiScale;
						containerSpriteCorner.y += (containerSprite.
							getTexture()->getSize().y / 2.0f) * guiScale;

						switch (inventoryMenu) {
						case InventoryMenu::Inventory:
							for (int craftingSlotIndex = 0; craftingSlotIndex < 4;
								craftingSlotIndex++)
							{
								const ItemContainer& itemContainer =
									craftingItemGrid[craftingSlotIndex];

								renderInventoryItem(itemContainer,
									getCraftingSlotPosition(craftingSlotIndex, false));
							}

							prepareText(
								0, gs::Vec2f(guiScale, guiScale) * 0.33f,
								gs::Color(60, 60, 65)
							);

							renderText(
								"Crafting", containerSpriteCorner 
									+ gs::Vec2f(96.0f, -158.0f) * guiScale
							); 

							break; 
						case InventoryMenu::Crafting:
							for (int craftingSlotIndex = 0; craftingSlotIndex < 9;
								craftingSlotIndex++)
							{
								const ItemContainer& itemContainer =
									craftingItemGrid[craftingSlotIndex];

								renderInventoryItem(itemContainer,
									getCraftingSlotPosition(craftingSlotIndex, true));
							}

							prepareText(
								0, gs::Vec2f(guiScale, guiScale) * 0.33f,
								gs::Color(60, 60, 65)
							);

							renderText(
								"Inventory", containerSpriteCorner 
									+ gs::Vec2f(8.0f, -93.0f) * guiScale
							);
							renderText(
								"Crafting", containerSpriteCorner 
									+ gs::Vec2f(28.0f, -160.0f) * guiScale
							);

							break;
						case InventoryMenu::Furnace:
						{
							TileEntity* tileEntity = 
								world->getTileEntity(tileEntityPosition); 

							if (tileEntity != nullptr) {
								for (int furnaceSlot = 0; furnaceSlot < 2; furnaceSlot++) {
									const ItemContainer& itemContainer = 
										tileEntity->getItemContainer(furnaceSlot); 

									renderInventoryItem(itemContainer, 
										getFurnaceInputPosition(furnaceSlot));
								}

								const ItemContainer& itemContainer =
									tileEntity->getItemContainer(2);

								renderInventoryItem(itemContainer,
									getFurnaceOutputPosition());

								const float fuelProgressScaler = static_cast<float>(
									tileEntity->tags.fuelRemaining)
										/ static_cast<float>(tileEntity->tags.fuelMax); 

								fuelProgressSprite.setScale(guiScale, guiScale); 
								fuelProgressSprite.setPosition(
									getFurnaceFuelProgressPosition()
								); 

								const int fuelProgressStartingPixel = (1.0f - fuelProgressScaler)
									* fuelProgressTexture.getSize().y; 

								fuelProgressSprite.setTextureRect(sf::IntRect(0,
									fuelProgressStartingPixel, 
									fuelProgressTexture.getSize().x, 
									fuelProgressTexture.getSize().y - fuelProgressStartingPixel
								));
								fuelProgressSprite.move(0.0, fuelProgressStartingPixel * guiScale); 

								window::winmain->draw(fuelProgressSprite); 

								const float smeltingProgressScaler = static_cast<float>(
									tileEntity->tags.smeltingProgress)
										/ static_cast<float>(TileEntity::smeltingDuration); 

								smeltingProgressSprite.setScale(guiScale, guiScale); 
								smeltingProgressSprite.setPosition(
									getFurnaceSmeltingProgressPosition()
								); 
								smeltingProgressSprite.setTextureRect(sf::IntRect(0, 0,
									static_cast<int>(smeltingProgressTexture.getSize().x 
										* smeltingProgressScaler), 
									static_cast<int>(smeltingProgressTexture.getSize().y)
								));

								window::winmain->draw(smeltingProgressSprite); 
							}

							prepareText(
								0, gs::Vec2f(guiScale, guiScale) * 0.33f,
								gs::Color(60, 60, 65)
							);

							renderText(
								"Inventory", containerSpriteCorner 
									+ gs::Vec2f(8.0f, -93.0f) * guiScale
							);
							renderText(
								"Furnace", containerSpriteCorner
									+ gs::Vec2f(72.0f, -160.0f) * guiScale
							);
						}
							break;
						case InventoryMenu::Chest:
						{
							TileEntity* tileEntity =
								world->getTileEntity(tileEntityPosition); 

							if (tileEntity != nullptr) {
								for (int chestSlot = 0; chestSlot <
									TileEntity::numOfChestSlots; chestSlot++)
								{
									const ItemContainer& itemContainer =
										tileEntity->getItemContainer(chestSlot); 

									renderInventoryItem(itemContainer,
										getChestSlotPosition(chestSlot)); 
								}
							}

							prepareText(
								0, gs::Vec2f(guiScale, guiScale) * 0.33f,
								gs::Color(60, 60, 65)
							);

							renderText(
								"Inventory", containerSpriteCorner
									+ gs::Vec2f(8.0f, -93.0f) * guiScale
							);
							renderText(
								"Chest", containerSpriteCorner
									+ gs::Vec2f(8.0f, -159.0f) * guiScale
							);
						}
							break;
						}

						// Render inventory items. 
						for (int inventoryIndex = 0; inventoryIndex <
							Player::numOfInventorySlots; inventoryIndex++)
						{
							const ItemContainer& itemContainer =
								player->inventory[inventoryIndex];  

							renderInventoryItem(itemContainer, 
								getInventorySlotPosition(inventoryIndex));
						}

						// Render recipe output if output exists. 
						if (outputRecipeContainer.count > 0) {
							const ItemContainer itemContainer(
								outputRecipeContainer.recipe.output, 
								outputRecipeContainer.recipe.count
							); 

							renderInventoryItem(itemContainer, 
								getCraftingOutputSlotPosition(fullTable));
						}

						// Renders item at mouse position, if one is held. 
						if (temporaryItemContainer.count > 0) {
							renderInventoryItem(temporaryItemContainer, 
								gs::input::mousePosition); 
						}
						else if (itemPreviewString != "")
							renderTextBox(itemPreviewString, 
								gs::input::mousePosition);
					}
				}

				if (gamePaused) {
					if (hideUIMenu) {
						if (screenShadePercentage > 0.0f) {
							screenShade.setFillColor(gs::util::approach(
								gs::Color::White, gs::Color::Black, screenShadePercentage
							));

							window::winmain->draw(screenShade, sf::BlendMultiply);
						}
					}

					renderPauseMenu();
				}

				if (gameOver) {
					renderGameOverMenu(); 
				}
			}
		}
	}
}