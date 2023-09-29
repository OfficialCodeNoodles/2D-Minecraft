#include "../../hdr/util/Input.hpp"
#include "../../hdr/graphics/UI.hpp"

namespace engine {
	namespace input {
		const std::string keyNames[sf::Keyboard::KeyCount] = {
			"A",					"B",
			"C",					"D",
			"E",					"F",
			"G",					"H",
			"I",					"J",
			"K",					"L",
			"M",					"N",
			"O",					"P",
			"Q",					"R",
			"S",					"T",
			"U",					"V",
			"W",					"X",
			"Y",					"Z",
			"Num0",					"Num1",
			"Num2",					"Num3",
			"Num4",					"Num5",
			"Num6",					"Num7",
			"Num8",					"Num9",
			"Escape",				"LControl",
			"LShift",				"LAlt",
			"LSystem",				"RControl",
			"RShift",				"RAlt",
			"RSystem",				"Menu",
			"LBracket",				"RBracket",
			"Semicolon",			"Comma",
			"Period",				"Quote",
			"Slash",				"Backslash",
			"Tilde",				"Equal",
			"Hyphen",				"Space",
			"Enter",				"Backspace",
			"Tab",					"PageUp",
			"PageDown",				"End",
			"Home",					"Insert",
			"Delete",				"Add",
			"Subtract",				"Multiply",
			"Divide",				"Left",
			"Right",				"Up",
			"Down",					"Numpad0",
			"Numpad1",				"Numpad2",
			"Numpad3",				"Numpad4",
			"Numpad5",				"Numpad6",
			"Numpad7",				"Numpad8",
			"Numpad9",				"F1",
			"F2",					"F3",
			"F4",					"F5",
			"F6",					"F7",
			"F8",					"F9",
			"F10",					"F11",
			"F12",					"F13",
			"F14",					"F15",
			"Pause"
		}; 

		sf::Keyboard::Key keyMap[End]; 
		bool keyLocks[End];
		bool keySwitches[End];
		int keyTicks[End];
		int keyTimeSinceLastPress[End];

		bool isKeyPressed(Key key) {
			return sf::Keyboard::isKeyPressed(keyMap[key]); 
		}
		bool isKeyClicked(Key key) {
			return keySwitches[key]; 
		}
		int getKeyTicks(Key key) {
			return keyTicks[key]; 
		}
		int getTimeSinceLastPress(Key key) {
			return keyTimeSinceLastPress[key];
		}

		void initKeys() {
			for (int keyIndex = 0; keyIndex < End; keyIndex++) {
				keyLocks[keyIndex] = false;	 
				keySwitches[keyIndex] = false; 
				keyTicks[keyIndex] = 0; 
				keyTimeSinceLastPress[keyIndex] = 0; 
			}

			initDefaultKeyBindings(); 
		}
		void initDefaultKeyBindings() {
			int keyIndex = 0; 
			for (auto& key : {
				sf::Keyboard::A,		sf::Keyboard::D,
				sf::Keyboard::W,		sf::Keyboard::S,
				sf::Keyboard::Space,	sf::Keyboard::LShift,
				sf::Keyboard::R,		sf::Keyboard::LControl,
				sf::Keyboard::Q,		sf::Keyboard::E,
				sf::Keyboard::Escape,	sf::Keyboard::Num1,
				sf::Keyboard::Num2,		sf::Keyboard::Num3,
				sf::Keyboard::Num4,		sf::Keyboard::Num5,
				sf::Keyboard::Num6,		sf::Keyboard::Num7,
				sf::Keyboard::Num8,		sf::Keyboard::Num9,
				sf::Keyboard::LShift,	sf::Keyboard::F1,
				sf::Keyboard::F2,		sf::Keyboard::F3,
				sf::Keyboard::F4,		sf::Keyboard::F5,
				sf::Keyboard::F6,		sf::Keyboard::F7,
				sf::Keyboard::F8}) 
			{
				keyMap[keyIndex] = key; 
				keyIndex++; 
			}

			for (int remappingButtonIndex = 0; remappingButtonIndex <
				render::ui::numOfRemappingButtons; remappingButtonIndex++) 
			{
				render::ui::RemappingButton& button = 
					render::ui::settingsControlsRemappingButtons[remappingButtonIndex]; 

				button.mappedKey = input::keyMap[button.key];
				button.mappedKeyName = input::keyNames[button.mappedKey];
			}
		}
		void updateKeys() {
			for (int keyIndex = 0; keyIndex < End; keyIndex++) {
				const sf::Keyboard::Key key = keyMap[keyIndex]; 
				const bool keyPressed = sf::Keyboard::isKeyPressed(key);

				if (!keySwitches[keyIndex])
					keyTimeSinceLastPress[keyIndex]++;
				else
					keyTimeSinceLastPress[keyIndex] = 0; 

				keySwitches[keyIndex] = !keyLocks[keyIndex] && keyPressed; 
				keyLocks[keyIndex] = keyPressed; 

				if (!keyLocks[keyIndex])
					keyTicks[keyIndex] = 0;
				else
					keyTicks[keyIndex]++; 
			}
		}
	}
}