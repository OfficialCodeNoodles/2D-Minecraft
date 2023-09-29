#pragma once

// Dependencies
#include "../Resources.hpp"

namespace engine {
	namespace input {
		extern const std::string keyNames[sf::Keyboard::KeyCount]; 

		enum Key {
			MoveLeft,				MoveRight,
			MoveUp,					MoveDown, 
			Jump,					Crouch, 
			SwitchTileMode,			Zoom, 
			DropItem,				OpenInventory, 
			MenuEscape,				Hotbar1, 
			Hotbar2,				Hotbar3,
			Hotbar4,				Hotbar5, 
			Hotbar6,				Hotbar7,
			Hotbar8,				Hotbar9,
			ItemShift,				HideUI, 
			TakeScreenshot,			OpenDebugMenu, 
			ChangeLightingStyle,	EnableFullBright, 
			ChangeFramerate,		EnterSpectatorMode,
			IncrementDayTime,
			End
		};

		extern sf::Keyboard::Key keyMap[End]; 
		extern bool keyLocks[End]; 
		extern bool keySwitches[End]; 
		extern int keyTicks[End]; 
		extern int keyTimeSinceLastPress[End]; 

		bool isKeyPressed(Key key); 
		bool isKeyClicked(Key key); 
		int getKeyTicks(Key key); 
		int getTimeSinceLastPress(Key key); 

		void initKeys(); 
		void initDefaultKeyBindings(); 
		void updateKeys(); 
	}
}