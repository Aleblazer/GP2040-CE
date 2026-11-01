#include "addons/shift_register_input.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"

bool ShiftRegisterInput::available() {
    const ShiftRegisterInputOptions& options = Storage::getInstance().getAddonOptions().shiftRegisterInputOptions;
    return (options.enabled &&
        isValidPin(options.latchPin) &&
        isValidPin(options.clockPin) &&
        isValidPin(options.dataPin) &&
        options.chainLength >= 1 && options.chainLength <= SHIFT_REGISTER_INPUT_MAX_CHIPS);
}

void ShiftRegisterInput::setup() {
    const ShiftRegisterInputOptions& options = Storage::getInstance().getAddonOptions().shiftRegisterInputOptions;

    latchPin = options.latchPin;
    clockPin = options.clockPin;
    dataPin = options.dataPin;
    inputCount = options.chainLength * SHIFT_REGISTER_INPUT_PINS_PER_CHIP;
    activeHigh = options.activeHigh;
    halfPeriodCycles = (uint32_t)(((uint64_t)clock_get_hz(clk_sys) * SHIFT_REGISTER_INPUT_HALF_PERIOD_NS) / 1000000000ULL);

    // SH/LD idles high (shift mode), CLK idles low
    gpio_init(latchPin);
    gpio_set_dir(latchPin, GPIO_OUT);
    gpio_put(latchPin, 1);

    gpio_init(clockPin);
    gpio_set_dir(clockPin, GPIO_OUT);
    gpio_put(clockPin, 0);

    // Bias the data line to "released" so a disconnected chain doesn't read as pressed
    gpio_init(dataPin);
    gpio_set_dir(dataPin, GPIO_IN);
    if (activeHigh) {
        gpio_pull_down(dataPin);
    } else {
        gpio_pull_up(dataPin);
    }

    // Tell the gamepad and the other add-ons which actions these inputs can trigger
    mappedInputs = 0;
    for (uint8_t input = 0; input < inputCount; input++) {
        inputMappings[input] = (input < options.pins_count) ? options.pins[input] : GpioMappingInfo{};
        GpioAction action = inputMappings[input].action;
        if (action == GpioAction::NONE || action == GpioAction::RESERVED || action == GpioAction::ASSIGNED_TO_ADDON) continue;

        mappedInputs |= (1u << input);
        forEachAction(inputMappings[input], &Gamepad::registerExternalAction);
    }

    debouncedInputs = readChain();
    Storage::getInstance().GetGamepad()->debouncedShiftRegisterInputs = debouncedInputs;
}

void ShiftRegisterInput::readInputs() {
    // Every input is debounced, including unmapped ones, so lights can follow any input
    uint32_t pressedInputs = readChain();

    // Same debounce rules as the direct GPIO buttons: a change is accepted when
    // the input hasn't changed within the configured debounce delay
    uint32_t debounceDelay = Storage::getInstance().getGamepadOptions().debounceDelay;
    uint32_t changedInputs = pressedInputs ^ debouncedInputs;
    if (debounceDelay == 0) {
        debouncedInputs = pressedInputs;
    } else if (changedInputs != 0) {
        uint32_t now = getMillis();
        while (changedInputs != 0) {
            uint8_t input = __builtin_ctz(changedInputs);
            if ((now - debounceTime[input]) >= debounceDelay) {
                debouncedInputs ^= (1u << input);
                debounceTime[input] = now;
            }
            changedInputs &= changedInputs - 1;
        }
    }

    Storage::getInstance().GetGamepad()->debouncedShiftRegisterInputs = debouncedInputs;

    uint32_t activeInputs = debouncedInputs & mappedInputs;
    while (activeInputs != 0) {
        forEachAction(inputMappings[__builtin_ctz(activeInputs)], &Gamepad::holdExternalAction);
        activeInputs &= activeInputs - 1;
    }
}

// Applies the input's action; a custom combo expands to its buttons the same way
// Gamepad::setup() maps a combo on a GPIO pin
void ShiftRegisterInput::forEachAction(const GpioMappingInfo& mapping, void (Gamepad::*apply)(GpioAction)) {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    if (mapping.action != GpioAction::CUSTOM_BUTTON_COMBO) {
        (gamepad->*apply)(mapping.action);
        return;
    }

    static const struct { uint8_t mask; GpioAction dpad; GpioAction digital; } comboDirections[] = {
        { GAMEPAD_MASK_UP,    GpioAction::BUTTON_PRESS_UP,    GpioAction::DIGITAL_DIRECTION_UP },
        { GAMEPAD_MASK_DOWN,  GpioAction::BUTTON_PRESS_DOWN,  GpioAction::DIGITAL_DIRECTION_DOWN },
        { GAMEPAD_MASK_LEFT,  GpioAction::BUTTON_PRESS_LEFT,  GpioAction::DIGITAL_DIRECTION_LEFT },
        { GAMEPAD_MASK_RIGHT, GpioAction::BUTTON_PRESS_RIGHT, GpioAction::DIGITAL_DIRECTION_RIGHT },
    };
    static const struct { uint32_t mask; GpioAction action; } comboButtons[] = {
        { GAMEPAD_MASK_B1, GpioAction::BUTTON_PRESS_B1 }, { GAMEPAD_MASK_B2, GpioAction::BUTTON_PRESS_B2 },
        { GAMEPAD_MASK_B3, GpioAction::BUTTON_PRESS_B3 }, { GAMEPAD_MASK_B4, GpioAction::BUTTON_PRESS_B4 },
        { GAMEPAD_MASK_L1, GpioAction::BUTTON_PRESS_L1 }, { GAMEPAD_MASK_R1, GpioAction::BUTTON_PRESS_R1 },
        { GAMEPAD_MASK_L2, GpioAction::BUTTON_PRESS_L2 }, { GAMEPAD_MASK_R2, GpioAction::BUTTON_PRESS_R2 },
        { GAMEPAD_MASK_S1, GpioAction::BUTTON_PRESS_S1 }, { GAMEPAD_MASK_S2, GpioAction::BUTTON_PRESS_S2 },
        { GAMEPAD_MASK_L3, GpioAction::BUTTON_PRESS_L3 }, { GAMEPAD_MASK_R3, GpioAction::BUTTON_PRESS_R3 },
        { GAMEPAD_MASK_A1, GpioAction::BUTTON_PRESS_A1 }, { GAMEPAD_MASK_A2, GpioAction::BUTTON_PRESS_A2 },
        { GAMEPAD_MASK_A3, GpioAction::BUTTON_PRESS_A3 }, { GAMEPAD_MASK_A4, GpioAction::BUTTON_PRESS_A4 },
    };

    for (const auto& direction : comboDirections) {
        if (mapping.customDpadMask & direction.mask) {
            (gamepad->*apply)(direction.dpad);
            (gamepad->*apply)(direction.digital);
        }
    }
    for (const auto& button : comboButtons) {
        if (mapping.customButtonMask & button.mask) {
            (gamepad->*apply)(button.action);
        }
    }
}

// Returns one bit per input, set when the input is pressed
uint32_t ShiftRegisterInput::readChain() {
    // Pulse SH/LD low to capture all inputs, then return to shift mode
    gpio_put(latchPin, 0);
    delayHalfPeriod();
    gpio_put(latchPin, 1);
    delayHalfPeriod();

    // QH presents input H of the first chip; every rising clock edge shifts the next
    // input (G ... A, then the next chip's H ... A) onto it
    uint32_t levels = 0;
    for (uint8_t bit = 0; bit < inputCount; bit++) {
        uint8_t input = (bit & ~0x07) | (7 - (bit & 0x07));
        if (gpio_get(dataPin)) {
            levels |= (1u << input);
        }
        gpio_put(clockPin, 1);
        delayHalfPeriod();
        gpio_put(clockPin, 0);
        delayHalfPeriod();
    }

    uint32_t inputMask = (inputCount >= 32) ? 0xFFFFFFFFu : ((1u << inputCount) - 1);
    return (activeHigh ? levels : ~levels) & inputMask;
}

void ShiftRegisterInput::delayHalfPeriod() {
    busy_wait_at_least_cycles(halfPeriodCycles);
}
