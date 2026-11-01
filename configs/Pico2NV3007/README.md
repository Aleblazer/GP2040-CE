# GP2040 Configuration for Raspberry Pi Pico 2 with an NV3007 display

Raspberry Pi Pico 2 (RP2350) leverless/stick controller with a 142x428 NV3007 color TFT bar display on SPI0 and a PIO USB host port.

## Main Pin Mapping Configuration

| Pico 2 Pin  | Label    | Action                         | GP2040 | Xinput | Switch  | PS3/4/5  |
|-------------|----------|--------------------------------|--------|--------|---------|----------|
| GPIO_PIN_16 | UP       | GpioAction::BUTTON_PRESS_UP    | UP     | UP     | UP      | UP       |
| GPIO_PIN_10 | DOWN     | GpioAction::BUTTON_PRESS_DOWN  | DOWN   | DOWN   | DOWN    | DOWN     |
| GPIO_PIN_11 | RIGHT    | GpioAction::BUTTON_PRESS_RIGHT | RIGHT  | RIGHT  | RIGHT   | RIGHT    |
| GPIO_PIN_09 | LEFT     | GpioAction::BUTTON_PRESS_LEFT  | LEFT   | LEFT   | LEFT    | LEFT     |
| GPIO_PIN_28 | CROSS    | GpioAction::BUTTON_PRESS_B1    | B1     | A      | B       | Cross    |
| GPIO_PIN_26 | CIRCLE   | GpioAction::BUTTON_PRESS_B2    | B2     | B      | A       | Circle   |
| GPIO_PIN_17 | SQUARE   | GpioAction::BUTTON_PRESS_B3    | B3     | X      | Y       | Square   |
| GPIO_PIN_27 | TRIANGLE | GpioAction::BUTTON_PRESS_B4    | B4     | Y      | X       | Triangle |
| GPIO_PIN_15 | L1       | GpioAction::BUTTON_PRESS_L1    | L1     | LB     | L       | L1       |
| GPIO_PIN_13 | R1       | GpioAction::BUTTON_PRESS_R1    | R1     | RB     | R       | R1       |
| GPIO_PIN_14 | L2       | GpioAction::BUTTON_PRESS_L2    | L2     | LT     | ZL      | L2       |
| GPIO_PIN_12 | R2       | GpioAction::BUTTON_PRESS_R2    | R2     | RT     | ZR      | R2       |
| GPIO_PIN_06 | OPT1     | GpioAction::BUTTON_PRESS_S1    | S1     | Back   | Minus   | Select   |
| GPIO_PIN_05 | OPT2     | GpioAction::BUTTON_PRESS_S2    | S2     | Start  | Plus    | Start    |
| GPIO_PIN_04 | OPT3     | GpioAction::BUTTON_PRESS_A1    | A1     | Guide  | Home    | PS       |
| GPIO_PIN_03 | OPT4     | GpioAction::BUTTON_PRESS_A2    | A2     | ~      | Capture | ~        |
| GPIO_PIN_02 | OPT5     | GpioAction::BUTTON_PRESS_L3    | L3     | LS     | LS      | L3       |
| GPIO_PIN_01 | OPT6     | GpioAction::BUTTON_PRESS_R3    | R3     | RS     | RS      | R3       |

The OPT1-OPT6 assignments are defaults and can be remapped on the Pin Mapping page.

## Display (NV3007, SPI0)

| Pico 2 Pin  | Signal |
|-------------|--------|
| GPIO_PIN_18 | SCK    |
| GPIO_PIN_19 | MOSI   |
| GPIO_PIN_00 | CS     |
| GPIO_PIN_07 | DC     |
| GPIO_PIN_08 | RESET  |
| GPIO_PIN_22 | BL (PWM, follows the Display Contrast setting) |

The standard GP2040-CE 128x64 screens are scaled up onto the 428x142 panel in landscape. Scaling, colors, panel offsets and the vendor init sequence can be changed under Display Configuration in the web configurator.

## USB Host

| Pico 2 Pin  | Signal |
|-------------|--------|
| GPIO_PIN_20 | D+     |
| GPIO_PIN_21 | D-     |
