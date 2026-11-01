#ifndef _ShiftRegisterInput_H
#define _ShiftRegisterInput_H

#include <string>
#include <stdint.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "enums.pb.h"

// Reads buttons from a chain of 74HC165 parallel-in/serial-out shift registers.
//
// Wiring: SH/LD of every chip to the latch pin, CLK of every chip to the clock pin,
// CLK INH tied low, and QH of the first chip to the data pin. Each further chip's QH
// feeds the SER input of the chip before it. Inputs are numbered per chip, A to H,
// with chip 1 being the one wired to the data pin. Buttons normally short an input
// to ground against a pull-up resistor (active low).

// ShiftRegisterInput Module Name
#define ShiftRegisterInputName "ShiftRegisterInput"

#define SHIFT_REGISTER_INPUT_MAX_CHIPS 4
#define SHIFT_REGISTER_INPUT_PINS_PER_CHIP 8
#define SHIFT_REGISTER_INPUT_MAX_PINS (SHIFT_REGISTER_INPUT_MAX_CHIPS * SHIFT_REGISTER_INPUT_PINS_PER_CHIP)

#ifndef SHIFT_REGISTER_INPUT_ENABLED
#define SHIFT_REGISTER_INPUT_ENABLED 0
#endif

#ifndef SHIFT_REGISTER_INPUT_LATCH_PIN
#define SHIFT_REGISTER_INPUT_LATCH_PIN -1
#endif

#ifndef SHIFT_REGISTER_INPUT_CLOCK_PIN
#define SHIFT_REGISTER_INPUT_CLOCK_PIN -1
#endif

#ifndef SHIFT_REGISTER_INPUT_DATA_PIN
#define SHIFT_REGISTER_INPUT_DATA_PIN -1
#endif

#ifndef SHIFT_REGISTER_INPUT_CHAIN_LENGTH
#define SHIFT_REGISTER_INPUT_CHAIN_LENGTH 1
#endif

#ifndef SHIFT_REGISTER_INPUT_ACTIVE_HIGH
#define SHIFT_REGISTER_INPUT_ACTIVE_HIGH 0
#endif

// Half of the shift clock period. 150ns leaves margin over the 74HC165's
// worst case clock-to-output delay when run from 3.3V.
#ifndef SHIFT_REGISTER_INPUT_HALF_PERIOD_NS
#define SHIFT_REGISTER_INPUT_HALF_PERIOD_NS 150
#endif

// Input action defaults, numbered (chip - 1) * 8 + input (A = 0 ... H = 7)
#ifndef SHIFT_REGISTER_INPUT_PIN00_ACTION
#define SHIFT_REGISTER_INPUT_PIN00_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN01_ACTION
#define SHIFT_REGISTER_INPUT_PIN01_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN02_ACTION
#define SHIFT_REGISTER_INPUT_PIN02_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN03_ACTION
#define SHIFT_REGISTER_INPUT_PIN03_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN04_ACTION
#define SHIFT_REGISTER_INPUT_PIN04_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN05_ACTION
#define SHIFT_REGISTER_INPUT_PIN05_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN06_ACTION
#define SHIFT_REGISTER_INPUT_PIN06_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN07_ACTION
#define SHIFT_REGISTER_INPUT_PIN07_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN08_ACTION
#define SHIFT_REGISTER_INPUT_PIN08_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN09_ACTION
#define SHIFT_REGISTER_INPUT_PIN09_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN10_ACTION
#define SHIFT_REGISTER_INPUT_PIN10_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN11_ACTION
#define SHIFT_REGISTER_INPUT_PIN11_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN12_ACTION
#define SHIFT_REGISTER_INPUT_PIN12_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN13_ACTION
#define SHIFT_REGISTER_INPUT_PIN13_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN14_ACTION
#define SHIFT_REGISTER_INPUT_PIN14_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN15_ACTION
#define SHIFT_REGISTER_INPUT_PIN15_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN16_ACTION
#define SHIFT_REGISTER_INPUT_PIN16_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN17_ACTION
#define SHIFT_REGISTER_INPUT_PIN17_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN18_ACTION
#define SHIFT_REGISTER_INPUT_PIN18_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN19_ACTION
#define SHIFT_REGISTER_INPUT_PIN19_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN20_ACTION
#define SHIFT_REGISTER_INPUT_PIN20_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN21_ACTION
#define SHIFT_REGISTER_INPUT_PIN21_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN22_ACTION
#define SHIFT_REGISTER_INPUT_PIN22_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN23_ACTION
#define SHIFT_REGISTER_INPUT_PIN23_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN24_ACTION
#define SHIFT_REGISTER_INPUT_PIN24_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN25_ACTION
#define SHIFT_REGISTER_INPUT_PIN25_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN26_ACTION
#define SHIFT_REGISTER_INPUT_PIN26_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN27_ACTION
#define SHIFT_REGISTER_INPUT_PIN27_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN28_ACTION
#define SHIFT_REGISTER_INPUT_PIN28_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN29_ACTION
#define SHIFT_REGISTER_INPUT_PIN29_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN30_ACTION
#define SHIFT_REGISTER_INPUT_PIN30_ACTION GpioAction::NONE
#endif

#ifndef SHIFT_REGISTER_INPUT_PIN31_ACTION
#define SHIFT_REGISTER_INPUT_PIN31_ACTION GpioAction::NONE
#endif

class ShiftRegisterInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // ShiftRegisterInput Setup
    virtual void readInputs();  // ShiftRegisterInput Read (before the gamepad state is built)
    virtual void process() {}   // ShiftRegisterInput Process
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return ShiftRegisterInputName; }
private:
    uint32_t readChain();
    void delayHalfPeriod();
    void forEachAction(const GpioMappingInfo& mapping, void (Gamepad::*apply)(GpioAction));

    uint8_t latchPin;
    uint8_t clockPin;
    uint8_t dataPin;
    uint8_t inputCount;
    bool activeHigh;
    uint32_t halfPeriodCycles;

    GpioMappingInfo inputMappings[SHIFT_REGISTER_INPUT_MAX_PINS];
    uint32_t mappedInputs = 0;

    uint32_t debouncedInputs = 0;
    uint32_t debounceTime[SHIFT_REGISTER_INPUT_MAX_PINS] = {};
};

#endif  // _ShiftRegisterInput_H
