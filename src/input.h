#pragma once

#include "olcPixelGameEngine.h"

#define OLC_PGE_GAMEPAD
#include "olcPGEX_Gamepad.h"

#include <map>

class Input
{
	std::map<std::string, olc::GPButtons> buttonMap;
	std::map<std::string, olc::Key> keyMap;

	std::string labels[10]
	{
		"Forward", "R_Left", "R_Right", "Shoot", "Block", "T_Forward", "T_Left", "T_Right", "T_Back", "Menu"
	};

	//olc::GPButtons curButtons[10];
	//olc::Key curKeys[10];

	olc::GPButtons defButtons[10]
	{
		olc::GPButtons::DPAD_U, olc::GPButtons::DPAD_L, olc::GPButtons::DPAD_R, olc::GPButtons::R1, olc::GPButtons::L1,
		olc::GPButtons::FACE_U, olc::GPButtons::FACE_L, olc::GPButtons::FACE_R, olc::GPButtons::FACE_D, olc::GPButtons::START
	};

	olc::Key defKeys[10]
	{
		olc::W, olc::A, olc::D, olc::U, olc::O, olc::I, olc::J, olc::L, olc::K, olc::ENTER
	};

	std::vector<std::string> keyNames =
	{
		"NONE",
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
		"K0", "K1", "K2", "K3", "K4", "K5", "K6", "K7", "K8", "K9",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"UP", "DOWN", "LEFT", "RIGHT",
		"SPACE", "TAB", "SHIFT", "CTRL", "INS", "DEL", "HOME", "END", "PGUP", "PGDN",
		"BACK", "ESCAPE", "RETURN", "ENTER", "PAUSE", "SCROLL",
		"NP0", "NP1", "NP2", "NP3", "NP4", "NP5", "NP6", "NP7", "NP8", "NP9",
		"NP_MUL", "NP_DIV", "NP_ADD", "NP_SUB", "NP_DECIMAL", "PERIOD",
		"EQUALS", "COMMA", "MINUS",
		"OEM_1", "OEM_2", "OEM_3", "OEM_4", "OEM_5", "OEM_6", "OEM_7", "OEM_8",
		"CAPS_LOCK", "ENUM_END"
	};

	std::vector<std::string> buttonNames =
	{
		"FACE_L", "FACE_D", "FACE_R", "FACE_U", "L1", "R1", "L2", "R2",
		"SELECT", "START", "L3", "R3", "12", "13", "DPAD_L", "DPAD_R", "DPAD_U", "DPAD_D"
	};

	olc::GamePad* gamepad = nullptr;
	olc::PixelGameEngine* pge;

public:
	Input(olc::PixelGameEngine* _pge_)
	{
		pge = _pge_;
		olc::GamePad::init();
		Initialize();
	}

	void Select()
	{
		if (gamepad == nullptr) gamepad = olc::GamePad::selectWithAnyButton();
	}

	void Initialize()
	{
		//buttonMap = new std::map<std::string, olc::GPButtons>();
		for (int i = 0; i < 10; i++)
		{
			buttonMap.emplace(labels[i], defButtons[i]);
			//curButtons[i] = buttonMap[labels[i]];
		}
		//keyMap = new std::map<std::string, olc::Key>();
		for (int i = 0; i < 10; i++)
		{
			keyMap.emplace(labels[i], defKeys[i]);
			//curKeys[i] = keyMap[labels[i]];
		}
	}

	std::string GetKeyName(std::string lbl)
	{
		return keyNames[static_cast<int>(keyMap[lbl])];
	}

	std::string GetButtonName(std::string lbl)
	{
		return buttonNames[static_cast<int>(buttonMap[lbl])];
	}

	bool CaptureKey(int idx)
	{
		for (int i = olc::NONE + 1; i != olc::ENUM_END; i++)
		{
			olc::Key k = static_cast<olc::Key>(i);
			if (pge->GetKey(k).bPressed)
			{
				for (auto& other : keyMap)
					if (other.second == k)
					{
						other.second = keyMap[labels[idx]];
						break;
					}
				keyMap[labels[idx]] = k;
				return true;
			}
		}
		return false;
	}

	bool CaptureButton(int idx)
	{
		if (gamepad == nullptr || !gamepad->stillConnected) return true;

		int start = static_cast<int>(olc::GPButtons::FACE_L);
		int end = static_cast<int>(olc::GPButtons::DPAD_D);
		for (int i = start; i <= end; i++)
		{
			olc::GPButtons b = static_cast<olc::GPButtons>(i);
			if (gamepad->getButton(b).bPressed)
			{
				for (auto& other : buttonMap)
					if (other.second == b)
					{
						other.second = buttonMap[labels[idx]];
						break;
					}
				buttonMap[labels[idx]] = b;
				return true;
			}
		}
		return false;
	}

	bool OnHold(std::string lbl)
	{
		return pge->GetKey(keyMap[lbl]).bHeld || (gamepad != nullptr && gamepad->stillConnected && gamepad->getButton(buttonMap[lbl]).bHeld);
	}

	bool OnPress(std::string lbl)
	{
		return pge->GetKey(keyMap[lbl]).bPressed || (gamepad != nullptr && gamepad->stillConnected && gamepad->getButton(buttonMap[lbl]).bPressed);
	}

	bool OnRelease(std::string lbl)
	{
		return pge->GetKey(keyMap[lbl]).bReleased || (gamepad != nullptr && gamepad->stillConnected && gamepad->getButton(buttonMap[lbl]).bReleased);
	}
};