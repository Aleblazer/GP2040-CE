/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// GP2040 Libraries
#include "gamepad.h"
#include "enums.pb.h"
#include "storagemanager.h"
#include "types.h"

#include "FlashPROM.h"
#include "CRC32.h"

#include "drivermanager.h"
#include "storagemanager.h"
#include "system.h"

// MUST BE DEFINED for mpgs
uint32_t getMillis() {
	return to_ms_since_boot(get_absolute_time());
}

uint64_t getMicro() {
	return to_us_since_boot(get_absolute_time());
}

Gamepad::Gamepad() :
	options(Storage::getInstance().getGamepadOptions())
	, hotkeyOptions(Storage::getInstance().getHotkeyOptions())
{
}

void Gamepad::setup()
{
	// Configure pin mapping
	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();

	mapDpadUp       = new GamepadButtonMapping(GAMEPAD_MASK_UP);
	mapDpadDown     = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
	mapDpadLeft     = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
	mapDpadRight    = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
	mapButtonB1     = new GamepadButtonMapping(GAMEPAD_MASK_B1);
	mapButtonB2     = new GamepadButtonMapping(GAMEPAD_MASK_B2);
	mapButtonB3     = new GamepadButtonMapping(GAMEPAD_MASK_B3);
	mapButtonB4     = new GamepadButtonMapping(GAMEPAD_MASK_B4);
	mapButtonL1     = new GamepadButtonMapping(GAMEPAD_MASK_L1);
	mapButtonR1     = new GamepadButtonMapping(GAMEPAD_MASK_R1);
	mapButtonL2     = new GamepadButtonMapping(GAMEPAD_MASK_L2);
	mapButtonR2     = new GamepadButtonMapping(GAMEPAD_MASK_R2);
	mapButtonS1     = new GamepadButtonMapping(GAMEPAD_MASK_S1);
	mapButtonS2     = new GamepadButtonMapping(GAMEPAD_MASK_S2);
	mapButtonL3     = new GamepadButtonMapping(GAMEPAD_MASK_L3);
	mapButtonR3     = new GamepadButtonMapping(GAMEPAD_MASK_R3);
	mapButtonA1     = new GamepadButtonMapping(GAMEPAD_MASK_A1);
	mapButtonA2     = new GamepadButtonMapping(GAMEPAD_MASK_A2);
	mapButtonA3     = new GamepadButtonMapping(GAMEPAD_MASK_A3);
	mapButtonA4     = new GamepadButtonMapping(GAMEPAD_MASK_A4);
	mapButtonE1     = new GamepadButtonMapping(GAMEPAD_MASK_E1);
	mapButtonE2     = new GamepadButtonMapping(GAMEPAD_MASK_E2);
	mapButtonE3     = new GamepadButtonMapping(GAMEPAD_MASK_E3);
	mapButtonE4     = new GamepadButtonMapping(GAMEPAD_MASK_E4);
	mapButtonE5     = new GamepadButtonMapping(GAMEPAD_MASK_E5);
	mapButtonE6     = new GamepadButtonMapping(GAMEPAD_MASK_E6);
	mapButtonE7     = new GamepadButtonMapping(GAMEPAD_MASK_E7);
	mapButtonE8     = new GamepadButtonMapping(GAMEPAD_MASK_E8);
	mapButtonE9     = new GamepadButtonMapping(GAMEPAD_MASK_E9);
	mapButtonE10    = new GamepadButtonMapping(GAMEPAD_MASK_E10);
	mapButtonE11    = new GamepadButtonMapping(GAMEPAD_MASK_E11);
	mapButtonE12    = new GamepadButtonMapping(GAMEPAD_MASK_E12);
	mapButtonFn     = new GamepadButtonMapping(AUX_MASK_FUNCTION);
	mapButtonDP     = new GamepadButtonMapping(SUSTAIN_DP_MODE_DP);
	mapButtonLS     = new GamepadButtonMapping(SUSTAIN_DP_MODE_LS);
	mapButtonRS     = new GamepadButtonMapping(SUSTAIN_DP_MODE_RS);
	mapDigitalUp    = new GamepadButtonMapping(GAMEPAD_MASK_UP);
	mapDigitalDown  = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
	mapDigitalLeft  = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
	mapDigitalRight = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
	mapAnalogLSXNeg = new GamepadButtonMapping(ANALOG_DIRECTION_LS_X_NEG);
	mapAnalogLSXPos = new GamepadButtonMapping(ANALOG_DIRECTION_LS_X_POS);
	mapAnalogLSYNeg = new GamepadButtonMapping(ANALOG_DIRECTION_LS_Y_NEG);
	mapAnalogLSYPos = new GamepadButtonMapping(ANALOG_DIRECTION_LS_Y_POS);
	mapAnalogRSXNeg = new GamepadButtonMapping(ANALOG_DIRECTION_RS_X_NEG);
	mapAnalogRSXPos = new GamepadButtonMapping(ANALOG_DIRECTION_RS_X_POS);
	mapAnalogRSYNeg = new GamepadButtonMapping(ANALOG_DIRECTION_RS_Y_NEG);
	mapAnalogRSYPos = new GamepadButtonMapping(ANALOG_DIRECTION_RS_Y_POS);
	map48WayMode    = new GamepadButtonMapping(SUSTAIN_4_8_WAY_MODE);
	mapFocusMode    = new GamepadButtonMapping(SUSTAIN_FOCUS_MODE);

	const auto assignActionToMaps = [&](GpioAction action, Pin_t pin) -> void {
		switch (action) {
			case GpioAction::BUTTON_PRESS_UP:	mapDpadUp->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_DOWN:	mapDpadDown->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_LEFT:	mapDpadLeft->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_RIGHT:	mapDpadRight->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_B1:	mapButtonB1->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_B2:	mapButtonB2->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_B3:	mapButtonB3->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_B4:	mapButtonB4->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_L1:	mapButtonL1->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_R1:	mapButtonR1->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_L2:	mapButtonL2->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_R2:	mapButtonR2->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_S1:	mapButtonS1->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_S2:	mapButtonS2->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_L3:	mapButtonL3->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_R3:	mapButtonR3->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_A1:	mapButtonA1->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_A2:	mapButtonA2->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_A3:	mapButtonA3->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_A4:	mapButtonA4->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E1:	mapButtonE1->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E2:	mapButtonE2->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E3:	mapButtonE3->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E4:	mapButtonE4->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E5:	mapButtonE5->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E6:	mapButtonE6->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E7:	mapButtonE7->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E8:	mapButtonE8->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E9:	mapButtonE9->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E10:	mapButtonE10->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E11:	mapButtonE11->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_E12:	mapButtonE12->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::BUTTON_PRESS_FN:	mapButtonFn->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::SUSTAIN_DP_MODE_DP:	mapButtonDP->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::SUSTAIN_DP_MODE_LS:	mapButtonLS->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::SUSTAIN_DP_MODE_RS:	mapButtonRS->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::DIGITAL_DIRECTION_UP:	mapDigitalUp->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::DIGITAL_DIRECTION_DOWN:	mapDigitalDown->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::DIGITAL_DIRECTION_LEFT:	mapDigitalLeft->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::DIGITAL_DIRECTION_RIGHT:	mapDigitalRight->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_X_NEG:	mapAnalogLSXNeg->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_X_POS:	mapAnalogLSXPos->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_Y_NEG:	mapAnalogLSYNeg->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_Y_POS:	mapAnalogLSYPos->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_X_NEG:	mapAnalogRSXNeg->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_X_POS:	mapAnalogRSXPos->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_Y_NEG:	mapAnalogRSYNeg->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_Y_POS:	mapAnalogRSYPos->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::SUSTAIN_4_8_WAY_MODE:	map48WayMode->pinMask |= Mask_t{1} << pin; break;
			case GpioAction::SUSTAIN_FOCUS_MODE: mapFocusMode->pinMask |= Mask_t{1} << pin; break;
			default:				break;
		}
	};

	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		if (pinMappings[pin].action == GpioAction::CUSTOM_BUTTON_COMBO) {
			forEachComboAction(pinMappings[pin], [&](GpioAction action) { assignActionToMaps(action, pin); });
		} else {
			assignActionToMaps(pinMappings[pin].action, pin);
		}
	}

	// Define our hotkey array
	hotkeys[0] = hotkeyOptions.hotkey01;
	hotkeys[1] = hotkeyOptions.hotkey02;
	hotkeys[2] = hotkeyOptions.hotkey03;
	hotkeys[3] = hotkeyOptions.hotkey04;
	hotkeys[4] = hotkeyOptions.hotkey05;
	hotkeys[5] = hotkeyOptions.hotkey06;
	hotkeys[6] = hotkeyOptions.hotkey07;
	hotkeys[7] = hotkeyOptions.hotkey08;
	hotkeys[8] = hotkeyOptions.hotkey09;
	hotkeys[9] = hotkeyOptions.hotkey10;
	hotkeys[10] = hotkeyOptions.hotkey11;
	hotkeys[11] = hotkeyOptions.hotkey12;
	hotkeys[12] = hotkeyOptions.hotkey13;
	hotkeys[13] = hotkeyOptions.hotkey14;
	hotkeys[14] = hotkeyOptions.hotkey15;
	hotkeys[15] = hotkeyOptions.hotkey16;
}

/**
 * @brief Undo setup().
 */
void Gamepad::reinit()
{
	delete mapDpadUp;
	delete mapDpadDown;
	delete mapDpadLeft;
	delete mapDpadRight;
	delete mapButtonB1;
	delete mapButtonB2;
	delete mapButtonB3;
	delete mapButtonB4;
	delete mapButtonL1;
	delete mapButtonR1;
	delete mapButtonL2;
	delete mapButtonR2;
	delete mapButtonS1;
	delete mapButtonS2;
	delete mapButtonL3;
	delete mapButtonR3;
	delete mapButtonA1;
	delete mapButtonA2;
	delete mapButtonA3;
	delete mapButtonA4;
	delete mapButtonE1;
	delete mapButtonE2;
	delete mapButtonE3;
	delete mapButtonE4;
	delete mapButtonE5;
	delete mapButtonE6;
	delete mapButtonE7;
	delete mapButtonE8;
	delete mapButtonE9;
	delete mapButtonE10;
	delete mapButtonE11;
	delete mapButtonE12;
	delete mapButtonFn;
	delete mapButtonDP;
	delete mapButtonLS;
	delete mapButtonRS;
	delete mapDigitalUp;
	delete mapDigitalDown;
	delete mapDigitalLeft;
	delete mapDigitalRight;
	delete mapAnalogLSXNeg;
	delete mapAnalogLSXPos;
	delete mapAnalogLSYNeg;
	delete mapAnalogLSYPos;
	delete mapAnalogRSXNeg;
	delete mapAnalogRSXPos;
	delete mapAnalogRSYNeg;
	delete mapAnalogRSYPos;
	delete map48WayMode;
	delete mapFocusMode;

	// reinitialize pin mappings
	this->setup();
}

void Gamepad::process()
{
	// NOTE: Inverted X/Y-axis must run before SOCD and Dpad processing
	if (options.invertXAxis) {
		bool left = (state.dpad & mapDpadLeft->buttonMask) != 0;
		bool right = (state.dpad & mapDpadRight->buttonMask) != 0;
		state.dpad &= ~(mapDpadLeft->buttonMask | mapDpadRight->buttonMask);
		if (left)
			state.dpad |= mapDpadRight->buttonMask;
		if (right)
			state.dpad |= mapDpadLeft->buttonMask;
	}

	if (options.invertYAxis) {
		bool up = (state.dpad & mapDpadUp->buttonMask) != 0;
		bool down = (state.dpad & mapDpadDown->buttonMask) != 0;
		state.dpad &= ~(mapDpadUp->buttonMask | mapDpadDown->buttonMask);
		if (up)
			state.dpad |= mapDpadDown->buttonMask;
		if (down)
			state.dpad |= mapDpadUp->buttonMask;
	}

	// 4-way before SOCD, might have better history without losing any coherent functionality
	if (options.fourWayMode ^ map48WayModeToggle) {
		state.dpad = (state.dpad & 0xF0) | filterToFourWayMode(state.dpad);
	}

	uint8_t currentDpadSnapshot = state.dpad;

	// stash digital-only dpad state for later
	uint8_t dpadOnlyMask = ((currentDpadSnapshot & 0xF0) >> 4);

	// and mask out the mode-specific mask
	uint8_t dpadModeMask = (currentDpadSnapshot & 0x0F);

	// set dpad back to dpad mode-specific state
	state.dpad = dpadModeMask;

	// and join both states before cleanup, but only if the mode is dpad
	// combine dpad mode directions and standalone digital directions
	if (activeDpadMode == DpadMode::DPAD_MODE_DIGITAL) {
		state.dpad |= dpadOnlyMask;
	}

	// clean up after yourself. nobody likes bad inputs.
	state.dpad = runSOCDCleaner(resolveSOCDMode(options), state.dpad);

	// since analog modes only care about the dpad mode inputs, set the dpad state to digital only dpad values
	switch (activeDpadMode)
	{
		case DpadMode::DPAD_MODE_LEFT_ANALOG:
			state.lx = dpadToAnalogX(state.dpad);
			state.ly = dpadToAnalogY(state.dpad);
			state.dpad = dpadOnlyMask;
			break;

		case DpadMode::DPAD_MODE_RIGHT_ANALOG:
			state.rx = dpadToAnalogX(state.dpad);
			state.ry = dpadToAnalogY(state.dpad);
			state.dpad = dpadOnlyMask;
			break;

		default:
			break;
	}
}

void Gamepad::read()
{
	Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
	// A mapped pin or an external input (see holdExternalAction) presses the action
	const auto pressed = [&](const GamepadButtonMapping* map, GpioAction action) -> bool {
		return (values & map->pinMask) || isExternalActionHeld(action);
	};

	// Get the midpoint value for the current mode
	uint16_t joystickMid = GAMEPAD_JOYSTICK_MID;
	if ( DriverManager::getInstance().getDriver() != nullptr ) {
		joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
	}

	state.aux = 0
		| pressed(mapButtonFn, GpioAction::BUTTON_PRESS_FN)   ? mapButtonFn->buttonMask : 0;

	state.dpad = 0
		| (pressed(mapDpadUp, GpioAction::BUTTON_PRESS_UP)       ? mapDpadUp->buttonMask              : 0)
		| (pressed(mapDpadDown, GpioAction::BUTTON_PRESS_DOWN)     ? mapDpadDown->buttonMask            : 0)
		| (pressed(mapDpadLeft, GpioAction::BUTTON_PRESS_LEFT)     ? mapDpadLeft->buttonMask            : 0)
		| (pressed(mapDpadRight, GpioAction::BUTTON_PRESS_RIGHT)    ? mapDpadRight->buttonMask           : 0)
		| (pressed(mapDigitalUp, GpioAction::DIGITAL_DIRECTION_UP)    ? (mapDigitalUp->buttonMask << 4)    : 0)
		| (pressed(mapDigitalDown, GpioAction::DIGITAL_DIRECTION_DOWN)  ? (mapDigitalDown->buttonMask << 4)  : 0)
		| (pressed(mapDigitalLeft, GpioAction::DIGITAL_DIRECTION_LEFT)  ? (mapDigitalLeft->buttonMask << 4)  : 0)
		| (pressed(mapDigitalRight, GpioAction::DIGITAL_DIRECTION_RIGHT) ? (mapDigitalRight->buttonMask << 4) : 0)
	;

	state.buttons = 0
		| (pressed(mapButtonB1, GpioAction::BUTTON_PRESS_B1)  ? mapButtonB1->buttonMask  : 0)
		| (pressed(mapButtonB2, GpioAction::BUTTON_PRESS_B2)  ? mapButtonB2->buttonMask  : 0)
		| (pressed(mapButtonB3, GpioAction::BUTTON_PRESS_B3)  ? mapButtonB3->buttonMask  : 0)
		| (pressed(mapButtonB4, GpioAction::BUTTON_PRESS_B4)  ? mapButtonB4->buttonMask  : 0)
		| (pressed(mapButtonL1, GpioAction::BUTTON_PRESS_L1)  ? mapButtonL1->buttonMask  : 0)
		| (pressed(mapButtonR1, GpioAction::BUTTON_PRESS_R1)  ? mapButtonR1->buttonMask  : 0)
		| (pressed(mapButtonL2, GpioAction::BUTTON_PRESS_L2)  ? mapButtonL2->buttonMask  : 0)
		| (pressed(mapButtonR2, GpioAction::BUTTON_PRESS_R2)  ? mapButtonR2->buttonMask  : 0)
		| (pressed(mapButtonS1, GpioAction::BUTTON_PRESS_S1)  ? mapButtonS1->buttonMask  : 0)
		| (pressed(mapButtonS2, GpioAction::BUTTON_PRESS_S2)  ? mapButtonS2->buttonMask  : 0)
		| (pressed(mapButtonL3, GpioAction::BUTTON_PRESS_L3)  ? mapButtonL3->buttonMask  : 0)
		| (pressed(mapButtonR3, GpioAction::BUTTON_PRESS_R3)  ? mapButtonR3->buttonMask  : 0)
		| (pressed(mapButtonA1, GpioAction::BUTTON_PRESS_A1)  ? mapButtonA1->buttonMask  : 0)
		| (pressed(mapButtonA2, GpioAction::BUTTON_PRESS_A2)  ? mapButtonA2->buttonMask  : 0)
		| (pressed(mapButtonA3, GpioAction::BUTTON_PRESS_A3)  ? mapButtonA3->buttonMask  : 0)
		| (pressed(mapButtonA4, GpioAction::BUTTON_PRESS_A4)  ? mapButtonA4->buttonMask  : 0)
		| (pressed(mapButtonE1, GpioAction::BUTTON_PRESS_E1)  ? mapButtonE1->buttonMask  : 0)
		| (pressed(mapButtonE2, GpioAction::BUTTON_PRESS_E2)  ? mapButtonE2->buttonMask  : 0)
		| (pressed(mapButtonE3, GpioAction::BUTTON_PRESS_E3)  ? mapButtonE3->buttonMask  : 0)
		| (pressed(mapButtonE4, GpioAction::BUTTON_PRESS_E4)  ? mapButtonE4->buttonMask  : 0)
		| (pressed(mapButtonE5, GpioAction::BUTTON_PRESS_E5)  ? mapButtonE5->buttonMask  : 0)
		| (pressed(mapButtonE6, GpioAction::BUTTON_PRESS_E6)  ? mapButtonE6->buttonMask  : 0)
		| (pressed(mapButtonE7, GpioAction::BUTTON_PRESS_E7)  ? mapButtonE7->buttonMask  : 0)
		| (pressed(mapButtonE8, GpioAction::BUTTON_PRESS_E8)  ? mapButtonE8->buttonMask  : 0)
		| (pressed(mapButtonE9, GpioAction::BUTTON_PRESS_E9)  ? mapButtonE9->buttonMask  : 0)
		| (pressed(mapButtonE10, GpioAction::BUTTON_PRESS_E10) ? mapButtonE10->buttonMask : 0)
		| (pressed(mapButtonE11, GpioAction::BUTTON_PRESS_E11) ? mapButtonE11->buttonMask : 0)
		| (pressed(mapButtonE12, GpioAction::BUTTON_PRESS_E12) ? mapButtonE12->buttonMask : 0)
	;

	// hold current dpad state regardless of input mode -> output, which is determined in process()
	state.dpadOriginal = state.dpad;

	// set the effective dpad mode based on settings + overrides
	if (pressed(mapButtonDP, GpioAction::SUSTAIN_DP_MODE_DP))	activeDpadMode = DpadMode::DPAD_MODE_DIGITAL;
	else if (pressed(mapButtonLS, GpioAction::SUSTAIN_DP_MODE_LS))	activeDpadMode = DpadMode::DPAD_MODE_LEFT_ANALOG;
	else if (pressed(mapButtonRS, GpioAction::SUSTAIN_DP_MODE_RS))	activeDpadMode = DpadMode::DPAD_MODE_RIGHT_ANALOG;
	else					activeDpadMode = options.dpadMode;

	map48WayModeToggle = pressed(map48WayMode, GpioAction::SUSTAIN_4_8_WAY_MODE);

	if (pressed(mapAnalogLSXNeg, GpioAction::ANALOG_DIRECTION_LS_X_NEG)) {
		state.lx = GAMEPAD_JOYSTICK_MIN;
	} else if (pressed(mapAnalogLSXPos, GpioAction::ANALOG_DIRECTION_LS_X_POS)) {
		state.lx = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.lx = joystickMid;
	}
	if (pressed(mapAnalogLSYNeg, GpioAction::ANALOG_DIRECTION_LS_Y_NEG)) {
		state.ly = GAMEPAD_JOYSTICK_MIN;
	} else if (pressed(mapAnalogLSYPos, GpioAction::ANALOG_DIRECTION_LS_Y_POS)) {
		state.ly = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.ly = joystickMid;
	}

	if (pressed(mapAnalogRSXNeg, GpioAction::ANALOG_DIRECTION_RS_X_NEG)) {
		state.rx = GAMEPAD_JOYSTICK_MIN;
	} else if (pressed(mapAnalogRSXPos, GpioAction::ANALOG_DIRECTION_RS_X_POS)) {
		state.rx = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.rx = joystickMid;
	}
	if (pressed(mapAnalogRSYNeg, GpioAction::ANALOG_DIRECTION_RS_Y_NEG)) {
		state.ry = GAMEPAD_JOYSTICK_MIN;
	} else if (pressed(mapAnalogRSYPos, GpioAction::ANALOG_DIRECTION_RS_Y_POS)) {
		state.ry = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.ry = joystickMid;
	}

	state.lt = 0;
	state.rt = 0;
}

void Gamepad::hotkey() {
	if (options.lockHotkeys)
		return;

	// Look for a hot-key
	bool hasHotkey = false;
	for(int i = 0; i < 16; i++) {
		if (pressedHotkey(hotkeys[i])) {
			processHotkeyAction(selectHotkey(hotkeys[i]));
			hasHotkey = true;
		}
	}
	if (hasHotkey == false ) {
		lastAction = HOTKEY_NONE;
	}
}

void Gamepad::clearState() {
	state.dpad = 0;
	state.buttons = 0;
	state.aux = 0;
	state.lx = GAMEPAD_JOYSTICK_MID;
	state.ly = GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = 0;
	state.rt = 0;
}

void Gamepad::clearRumbleState() {
	auxState.haptics.leftActuator.active = false;
	auxState.haptics.leftActuator.intensity = 0;
	auxState.haptics.rightActuator.active = false;
	auxState.haptics.rightActuator.intensity = 0;
	auxState.haptics.leftTrigger.active = false;
	auxState.haptics.leftTrigger.intensity = 0;
	auxState.haptics.rightTrigger.active = false;
	auxState.haptics.rightTrigger.intensity = 0;
}

/**
 * @brief Take a hotkey action if it hasn't already been taken, modifying state/options appropriately.
 */
void Gamepad::processHotkeyAction(GamepadHotkey action) {
	bool reqSave = false;
	switch (action) {
		case HOTKEY_DPAD_DIGITAL:
			if (action != lastAction) {
				options.dpadMode = DPAD_MODE_DIGITAL;
				reqSave = true;
			}
			break;
		case HOTKEY_DPAD_LEFT_ANALOG:
			if (action != lastAction) {
				options.dpadMode = DPAD_MODE_LEFT_ANALOG;
				reqSave = true;
			}
			break;
		case HOTKEY_DPAD_RIGHT_ANALOG:
			if (action != lastAction) {
				options.dpadMode = DPAD_MODE_RIGHT_ANALOG;
				reqSave = true;
			}
			break;
		case HOTKEY_HOME_BUTTON:
			state.buttons |= GAMEPAD_MASK_A1;
			break;
		case HOTKEY_L3_BUTTON:
			state.buttons |= GAMEPAD_MASK_L3;
			break;
		case HOTKEY_R3_BUTTON:
			state.buttons |= GAMEPAD_MASK_R3;
			break;
		case HOTKEY_B1_BUTTON:
			state.buttons |= GAMEPAD_MASK_B1;
			break;
		case HOTKEY_B2_BUTTON:
			state.buttons |= GAMEPAD_MASK_B2;
			break;
		case HOTKEY_B3_BUTTON:
			state.buttons |= GAMEPAD_MASK_B3;
			break;
		case HOTKEY_B4_BUTTON:
			state.buttons |= GAMEPAD_MASK_B4;
			break;
		case HOTKEY_L1_BUTTON:
			state.buttons |= GAMEPAD_MASK_L1;
			break;
		case HOTKEY_R1_BUTTON:
			state.buttons |= GAMEPAD_MASK_R1;
			break;
		case HOTKEY_L2_BUTTON:
			state.buttons |= GAMEPAD_MASK_L2;
			break;
		case HOTKEY_R2_BUTTON:
			state.buttons |= GAMEPAD_MASK_R2;
			break;
		case HOTKEY_S1_BUTTON:
			state.buttons |= GAMEPAD_MASK_S1;
			break;
		case HOTKEY_S2_BUTTON:
			state.buttons |= GAMEPAD_MASK_S2;
			break;
		case HOTKEY_A1_BUTTON:
			state.buttons |= GAMEPAD_MASK_A1;
			break;
		case HOTKEY_A2_BUTTON:
			state.buttons |= GAMEPAD_MASK_A2;
			break;
		case HOTKEY_A3_BUTTON:
			state.buttons |= GAMEPAD_MASK_A3;
			break;
		case HOTKEY_A4_BUTTON:
			state.buttons |= GAMEPAD_MASK_A4;
			break;
		case HOTKEY_DPAD_UP:
			state.dpad |= GAMEPAD_MASK_UP;
			break;
		case HOTKEY_DPAD_DOWN:
			state.dpad |= GAMEPAD_MASK_DOWN;
			break;
		case HOTKEY_DPAD_LEFT:
			state.dpad |= GAMEPAD_MASK_LEFT;
			break;
		case HOTKEY_DPAD_RIGHT:
			state.dpad |= GAMEPAD_MASK_RIGHT;
			break;
		case HOTKEY_SOCD_UP_PRIORITY:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_UP_PRIORITY;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_NEUTRAL:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_NEUTRAL;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_LAST_INPUT:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_FIRST_INPUT:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_FIRST_INPUT_PRIORITY;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_BYPASS:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_BYPASS;
				reqSave = true;
			}
			break;
		case HOTKEY_REBOOT_DEFAULT:
			if (action != lastAction) {
				EventManager::getInstance().triggerEvent(new GPRestartEvent((System::BootMode)System::BootMode::DEFAULT));
			}
			break;
		case HOTKEY_REBOOT_WEBCONFIG:
			if (action != lastAction) {
				EventManager::getInstance().triggerEvent(new GPRestartEvent((System::BootMode)System::BootMode::WEBCONFIG));
			}
			break;
		case HOTKEY_REBOOT_USB:
			if (action != lastAction) {
				EventManager::getInstance().triggerEvent(new GPRestartEvent((System::BootMode)System::BootMode::USB));
			}
			break;
		case HOTKEY_SAVE_CONFIG:
			if (action != lastAction) {
				Storage::getInstance().save(true);
			}
			break;
		case HOTKEY_CAPTURE_BUTTON:
			state.buttons |= GAMEPAD_MASK_A2;
			break;
		case HOTKEY_TOUCHPAD_BUTTON:
			state.buttons |= GAMEPAD_MASK_A2;
			break;
		case HOTKEY_INVERT_X_AXIS:
			if (action != lastAction) {
				options.invertXAxis = !options.invertXAxis;
				reqSave = true;
			}
			break;
		case HOTKEY_INVERT_Y_AXIS:
			if (action != lastAction) {
				options.invertYAxis = !options.invertYAxis;
				reqSave = true;
			}
			break;
		case HOTKEY_TOGGLE_4_WAY_MODE:
			if (action != lastAction) {
				options.fourWayMode = !options.fourWayMode;
				reqSave = true;
			}
			break;
		case HOTKEY_ENABLE_4_WAY_MODE:
			if (action != lastAction) {
				options.fourWayMode = true;
				reqSave = true;
			}
			break;
		case HOTKEY_DISABLE_4_WAY_MODE:
			if (action != lastAction) {
				options.fourWayMode = false;
				reqSave = true;
			}
			break;
		case HOTKEY_TOGGLE_DDI_4_WAY_MODE:
			if (action != lastAction) {
				DualDirectionalOptions& ddiOpt = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
				ddiOpt.fourWayMode = !ddiOpt.fourWayMode;
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_1:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(1)) {
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_2:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(2)) {
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_3:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(3)) {
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_4:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(4)) {
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_5:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(5)) {
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_6:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(6)) {
					reqSave = true;
				}
			}
			break;
		case HOTKEY_NEXT_PROFILE:
			if (action != lastAction) {
				Storage::getInstance().nextProfile();
				reqSave = true;
			}
			break;
		case HOTKEY_PREVIOUS_PROFILE:
			if (action != lastAction) {
				Storage::getInstance().previousProfile();
				reqSave = true;
			}
			break;
		case HOTKEY_TURBO_COUNT_UP:
			if (action != lastAction) {
				TurboOptions &turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
				turboOptions.shotCount++;
			}
			break;
		case HOTKEY_TURBO_COUNT_DOWN:
			if (action != lastAction) {
				TurboOptions &turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
				turboOptions.shotCount--;
			}
			break;
		case HOTKEY_MENU_NAV_UP:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_UP));
            }
			break;
		case HOTKEY_MENU_NAV_DOWN:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_DOWN));
            }
			break;
		case HOTKEY_MENU_NAV_LEFT:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_LEFT));
            }
			break;
		case HOTKEY_MENU_NAV_RIGHT:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_RIGHT));
            }
			break;
		case HOTKEY_MENU_NAV_SELECT:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_SELECT));
            }
			break;
		case HOTKEY_MENU_NAV_BACK:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_BACK));
            }
			break;
		case HOTKEY_MENU_NAV_TOGGLE:
			if (action != lastAction) {
				EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_TOGGLE));
			}
			break;
		case HOTKEY_FOCUS_MODE_TOGGLE:
		{
			auto &focusModeOptions = Storage::getInstance().getAddonOptions().focusModeOptions;

			if( !focusModeOptions.overrideEnabled )
			{
				if (action != lastAction)
				{
					focusModeOptions.overrideEnabled = true;
					reqSave = true;
				}
			}
			else // override is already enabled, hold action to disable
			{
				//start timer if last action was not HOTKEY_FOCUS_MODE_TOGGLE (i.e.. it has not been held), or
				//if the timer has not yet been started
				if( lastAction != action || is_nil_time(disableFocusModeTimeout))
				{
					constexpr uint32_t focusModeToggleHoldMs = 2000;
					disableFocusModeTimeout = make_timeout_time_ms(focusModeToggleHoldMs);
				}

				if( time_reached( disableFocusModeTimeout ) )
				{
					disableFocusModeTimeout = nil_time;
					focusModeOptions.overrideEnabled = false;
					reqSave = true;
				}
			}
		}
			break;
		case HOTKEY_LS_UP:
			state.ly = GAMEPAD_JOYSTICK_MIN;
			break;
		case HOTKEY_LS_DOWN:
			state.ly = GAMEPAD_JOYSTICK_MAX;
			break;
		case HOTKEY_LS_LEFT:
			state.lx = GAMEPAD_JOYSTICK_MIN;
			break;
		case HOTKEY_LS_RIGHT:
			state.lx = GAMEPAD_JOYSTICK_MAX;
			break;
		case HOTKEY_RS_UP:
			state.ry = GAMEPAD_JOYSTICK_MIN;
			break;
		case HOTKEY_RS_DOWN:
			state.ry = GAMEPAD_JOYSTICK_MAX;
			break;
		case HOTKEY_RS_LEFT:
			state.rx = GAMEPAD_JOYSTICK_MIN;
			break;
		case HOTKEY_RS_RIGHT:
			state.rx = GAMEPAD_JOYSTICK_MAX;
			break;
       
		default: // Unknown action
			break;
	}

	// only save if requested
	if (reqSave) {
		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	}

	lastAction = action;
}
