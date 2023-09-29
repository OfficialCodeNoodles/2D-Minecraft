#pragma once

// Dependencies
#include "Render.hpp"
#include "../inventory/Crafting.hpp"

namespace engine {
	namespace render {
		namespace ui {
			struct RemappingButton : public gs::Button {
				input::Key key; 
				std::string keyName; 
				sf::Keyboard::Key mappedKey; 
				std::string mappedKeyName; 

				RemappingButton();
				RemappingButton(input::Key key, const std::string& keyName); 
				~RemappingButton() = default;
			};

			constexpr int numOfFonts = 2; 
			constexpr int numOfWidgetTextures = 5; 
			constexpr int numOfRemappingButtons = 11; 

			extern sf::Font fonts[numOfFonts]; 
			extern gs::Text text; 
			extern float textBackgroundThickness; 
			extern gs::Color textBackgroundColor; 
			extern bool textSharpen; 
			extern sf::FloatRect prvsTextBounds; 
			extern bool displayDebugMenu; 
			extern bool hideUIMenu; 
			extern std::string fpsString; 
			extern sf::Texture cursorTexture;
			extern sf::Sprite cursorSprite; 
			extern sf::Texture widgetTextures[numOfWidgetTextures];
			extern sf::Sprite widgetSprite; 
			
			void prepareText(
				int fontId, gs::Vec2f scale = gs::Vec2f(1.0f, 1.0f),
				gs::Color fillColor = gs::Color::White,
				float outlineThickness = 0.0f,
				gs::Color outlineColor = gs::Color(),
				float backgroundThickness = 0.0f,
				gs::Color backgroundColor = gs::Color::Transparent, 
				bool sharpen = false
			); 
			void renderText(
				const std::string& string, gs::Vec2f position, int fontId, 
				bool centered = false, 	
				gs::Vec2f scale = gs::Vec2f(1.0f, 1.0f), 
				gs::Color fillColor = gs::Color::White, 
				float outlineThickness = 0.0f, 
				gs::Color outlineColor = gs::Color(),
				float backgroundThickness = 0.0f,
				gs::Color backgroundColor = gs::Color::Transparent,
				bool sharpen = false
			);
			void renderText(
				const std::string& string, gs::Vec2f position, 
				bool centered = false, bool ignoreRendering = false
			); 
			void prepareMenuButton(gs::Button& button); 
			void renderMenuButton(
				gs::Button& button, bool empty = false, 
				bool keepWhiteText = false	
			); 
			void prepareControlRemappingButton(RemappingButton& button); 
			void renderControlRemappingButton(
				RemappingButton& button, bool selected
			); 
			void prepareMenuSlider(gs::Button& button, gs::Slider& slider); 
			void renderMenuSlider(gs::Button& button, gs::Slider& slider);

			enum class InventoryMenu {
				None = -1, Inventory, Crafting, Furnace, Chest
			};
			enum class SettingsState {
				Main, VideoSettings, Controls
			};

			constexpr int numOfXpBarTextures = 2; 
			constexpr int numOfHeartTextures = 3; 
			constexpr int numOfHungerTextures = 3; 
			constexpr int numOfSwitcherTextures = 2; 
			constexpr int numOfContainerTextures = 4; 
			extern const gs::Vec2f worldPreviewSize; 
			extern const gs::Vec2i worldPreviewIconSize; 
			extern const int maxWorldNameLength; 
			extern const float tileSelectionOutlineThickness;
			extern const gs::Vec2f inventorySlotSize; 

			extern float guiScale; 
			extern sf::RectangleShape screenShade; 
			extern float screenShadePercentage; 
			extern float titleIntroTransparencyPercentage; 
			extern GameTime titleGameTime; 
			extern int titleBackgroundIndex; 
			extern sf::Texture titleIntroLogoTexture; 
			extern sf::Sprite titleIntroLogoSprite; 
			extern sf::RectangleShape titleIntroProgressBar;  
			extern sf::RectangleShape titleIntroProgressBarOutline;
			extern sf::Text tileIntroLoadingText; 
			extern sf::Texture titleLogoTexture; 
			extern sf::Sprite titleLogoSprite; 
			extern sf::Texture titleBlurTexture; 
			extern sf::Sprite titleBlurSprite; 
			extern std::vector<std::string> titleSplashTexts; 
			extern gs::Text titleSplashText; 
			extern int titleSplashIndex; 
			extern gs::Button titleSinglePlayerButton; 
			extern gs::Button titleChangeLogButton; 
			extern gs::Button titleCreditsButton; 
			extern gs::Button titleOptionsButton; 
			extern gs::Button titleQuitGameButton; 
			extern gs::Button titleYouTubeButton; 
			extern sf::RectangleShape titleYouTubeUnderline; 
			extern sf::Texture optionsBackgroundTexture; 
			extern sf::Sprite optionsBackgroundSprite; 
			extern sf::Texture missingWorldTexture; 
			extern sf::Sprite missingWorldSprite; 
			extern gs::Button worldSelectionPlayButton; 
			extern gs::Button worldSelectionEditButton; 
			extern gs::Button worldSelectionDeleteButton; 
			extern gs::Button worldSelectionCreateButton; 
			extern gs::Button worldSelectionCancelButton; 
			extern gs::Slider worldSelectionSlider; 
			extern float worldPreviewOffset; 
			extern sf::RectangleShape worldPreviewOutline; 
			extern int worldPreviewSelected;  
			extern gs::Textbox worldEditNameTextbox; 
			extern gs::Button worldEditResetIconButton; 
			extern gs::Button worldEditCancelButton; 
			extern gs::Button worldEditSaveButton; 
			extern gs::Textbox worldCreateWorldNameTextbox; 
			extern gs::Textbox worldCreateWorldSeedTextbox; 
			extern gs::Button worldCreateNewWorldButton; 
			extern gs::Button worldCreateCancelButton; 
			extern gs::Button changeLogDoneButton; 
			extern SettingsState settingsState; 
			extern gs::Button settingsDoneButton; 
			extern gs::Button settingsMusicVolumeButton; 
			extern gs::Slider settingsMusicVolumeSlider; 
			extern gs::Button settingsSoundVolumeButton; 
			extern gs::Slider settingsSoundVolumeSlider; 
			extern gs::Button settingsVideoSettingsButton;
			extern gs::Button settingsControlsButton;
			extern gs::Button settingsVideoSettingsFramerateButton; 
			extern gs::Button settingsVideoSettingsLightingStyleButton; 
			extern gs::Button settingsVideoSettingsRenderBiomeBackgroundButton; 
			extern gs::Button settingsVideoSettingsRenderSunAndMoonButton; 
			extern gs::Button settingsVideoSettingsRenderParticlesButton; 
			extern gs::Button settingsVideoSettingsRenderStarsButton; 
			extern gs::Button settingsVideoSettingsGuiScaleButton; 
			extern gs::Slider settingsVideoSettingsGuiScaleSlider; 
			extern gs::Button settingsVideoSettingsDoneButton; 
			extern int remappingButtonSelected; 
			extern RemappingButton settingsControlsRemappingButtons[numOfRemappingButtons]; 
			extern gs::Slider settingsControlsSlider; 
			extern gs::Button settingsControlsResetKeysButton; 
			extern gs::Button settingsControlsDoneButton; 
			extern sf::Texture itemAtlas; 
			extern sf::Sprite itemSprite; 
			extern Block::Id debugBlockId; 
			extern Wall::Id debugWallId; 
			extern bool useDebugBlock; 
			extern bool gamePaused; 
			extern bool pausedSettingsOpen; 
			extern gs::Button pauseBackToGameButton;
			extern gs::Button pauseSettingsButton; 
			extern gs::Button pauseSaveAndQuitButton; 
			extern gs::Button gameOverRespawnButton; 
			extern gs::Button gameOverTitleMenuButton; 
			extern bool shouldCrosshairBeRendered; 
			extern sf::RectangleShape tileSelectionOutline; 
			extern bool useBlock; 
			extern int hotbarIndex, prvsHotbarIndex; 
			extern sf::Texture hotbarTexture; 
			extern sf::Sprite hotbarSprite; 
			extern sf::Texture hotbarSelectorTexture;
			extern sf::Sprite hotbarSelectorSprite; 
			extern sf::Texture xpBarTextures[numOfXpBarTextures]; 
			extern sf::Sprite xpBarSprite; 
			extern sf::Texture heartTextures[numOfHeartTextures]; 
			extern sf::Sprite heartSprite; 
			extern sf::Texture hungerTextures[numOfHungerTextures]; 
			extern sf::Sprite hungerSprite; 
			extern sf::Texture switcherTextures[numOfSwitcherTextures]; 
			extern sf::Sprite switcherSprite; 
			extern InventoryMenu inventoryMenu; 
			extern bool inventoryOpen, prvsInventoryOpen; 
			extern bool gameOver; 
			extern ItemContainer temporaryItemContainer;
			extern sf::Texture containerTextures[numOfContainerTextures]; 
			extern sf::Sprite containerSprite; 
			extern sf::Texture fuelProgressTexture; 
			extern sf::Sprite fuelProgressSprite; 
			extern sf::Texture smeltingProgressTexture; 
			extern sf::Sprite smeltingProgressSprite; 
			extern std::string itemPreviewString; 
			extern int inventorySlotSelected; 
			extern std::string selectedItemName; 
			extern float selectedItemNameTimeRemaining;
			extern int lastInventorySlotSelected;
			extern gs::Vec2f inventorySlotShadePosition; 
			extern sf::RectangleShape inventorySlotShade; 
			extern ItemContainer craftingItemGrid[numOfCraftingGridSquares];
			extern int craftingSlotSelected; 
			extern ItemContainer craftingOutput;
			extern bool craftingOutputSelected; 
			extern RecipeContainer outputRecipeContainer; 
			extern int furnaceSlotSelected; 
			extern bool furnaceOutputSelected; 
			extern int chestSlotSelected; 

			void loadAssets(); 
			void handleAssets(); 
			gs::Vec2f getWorldPreviewPosition(int worldPreviewIndex); 
			gs::Vec2f getInventorySlotPosition(int inventorySlotIndex); 
			gs::Vec2f getCraftingSlotPosition(int craftingSlotIndex, bool fullTable = false); 
			gs::Vec2f getCraftingOutputSlotPosition(bool fullTable = false); 
			gs::Vec2f getFurnaceInputPosition(int furnaceSlot); 
			gs::Vec2f getFurnaceOutputPosition(); 
			gs::Vec2f getFurnaceFuelProgressPosition(); 
			gs::Vec2f getFurnaceSmeltingProgressPosition(); 
			gs::Vec2f getChestSlotPosition(int chestSlotIndex); 

			void renderTileBreaking(); 
			void renderTilePreview(gs::Vec2i position, sf::Sprite& sprite); 
			void renderBlockPreview(Block::Id blockId, gs::Vec2i position); 
			void renderWallPreview(Wall::Id wallId, gs::Vec2i position); 
			void renderItem(
				const Item& item, gs::Vec2f position, float size, 
				sf::RenderTarget& target = *window::winmain
			);
			void renderInventoryItem(
				const ItemContainer& itemContainer, gs::Vec2f position
			); 
			void renderTextBox(const std::string& name, gs::Vec2f position); 

			void updateLoadingScreen(); 
			void updateTitleScreen(); 
			void updateWorldSelectionScreen(); 
			void updateWorldEditScreen(); 
			void updateWorldCreateScreen(); 
			void updateChangeLogScreen(); 
			void updateCreditsScreen(); 
			void updateSettingsScreen(); 
			void updateDebugMenu();
			void updatePauseMenu(); 
			void updateGameOverMenu(); 
			void updateCrosshair(); 
			void updateIngameUI(); 
			
			void renderLoadingScreen(); 
			void renderTitleScreen(); 
			void renderMenuBackground(bool multiLayer = true, int layer = -1);
			void renderWorldPreview(const WorldPreview& worldPreview, int index); 
			void renderWorldSelectionScreen(); 
			void renderWorldEditScreen(); 
			void renderWorldCreateScreen(); 
			void renderChangeLogScreen(); 
			void renderCreditsScreen(); 
			void renderSettingsScreen(); 
			void renderDebugMenu(); 
			void renderPauseMenu(); 
			void renderGameOverMenu(); 
			void renderCrosshair(); 
			void renderTileSelectionOutline(); 
			void renderIngameUI(); 
		}
	}
}