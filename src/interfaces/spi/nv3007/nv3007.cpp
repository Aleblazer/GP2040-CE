#include "nv3007.h"

#include <algorithm>

#include "storagemanager.h"
#include "config.pb.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// Init sequences are encoded as: command, data length, data bytes...
// Command 0x00 (NOP) is never sent with data, so it is reused for control entries.
#define NV3007_SEQ_DELAY(ms) 0x00, 0xFE, (ms)
#define NV3007_SEQ_END 0x00, 0xFF

// Vendor sequence for the 142x428 NV3007 modules (1.68"/2.25" class), from Arduino_GFX
static const uint8_t NV3007_INIT_STANDARD[] = {
    0xFF, 1, 0xA5,
    0x11, 0,
    NV3007_SEQ_DELAY(120),
    0xFF, 1, 0xA5,
    0x9A, 1, 0x08,
    0x9B, 1, 0x08,
    0x9C, 1, 0xB0,
    0x9D, 1, 0x17,
    0x9E, 1, 0xC2,
    0x8F, 2, 0x22, 0x04,
    0x84, 1, 0x90,
    0x83, 1, 0x7B,
    0x85, 1, 0x4F,
    // Gamma
    0x6E, 1, 0x0F,
    0x7E, 1, 0x0F,
    0x60, 1, 0x00,
    0x70, 1, 0x00,
    0x6D, 1, 0x39,
    0x7D, 1, 0x31,
    0x61, 1, 0x0A,
    0x71, 1, 0x0A,
    0x6C, 1, 0x35,
    0x7C, 1, 0x29,
    0x62, 1, 0x0F,
    0x72, 1, 0x0F,
    0x68, 1, 0x4F,
    0x78, 1, 0x45,
    0x66, 1, 0x33,
    0x76, 1, 0x33,
    0x6B, 1, 0x14,
    0x7B, 1, 0x14,
    0x63, 1, 0x09,
    0x73, 1, 0x09,
    0x6A, 1, 0x13,
    0x7A, 1, 0x16,
    0x64, 1, 0x08,
    0x74, 1, 0x08,
    0x69, 1, 0x07,
    0x79, 1, 0x0D,
    0x65, 1, 0x05,
    0x75, 1, 0x05,
    0x67, 1, 0x33,
    0x77, 1, 0x33,
    0x6F, 1, 0x00,
    0x7F, 1, 0x00,
    0x50, 1, 0x00,
    0x52, 1, 0xD6,
    0x53, 1, 0x04,
    0x54, 1, 0x04,
    0x55, 1, 0x1B,
    0x56, 1, 0x1B,
    0xA0, 3, 0x2A, 0x24, 0x00,
    0xA1, 1, 0x84,
    0xA2, 1, 0x85,
    0xA8, 1, 0x34,
    0xA9, 1, 0x80,
    0xAA, 1, 0x73,
    0xAB, 2, 0x03, 0x61,
    0xAC, 2, 0x03, 0x65,
    0xAD, 2, 0x03, 0x60,
    0xAE, 2, 0x03, 0x64,
    0xB9, 1, 0x82,
    0xBA, 1, 0x83,
    0xBB, 1, 0x80,
    0xBC, 1, 0x81,
    0xBD, 1, 0x02,
    0xBE, 1, 0x01,
    0xBF, 1, 0x04,
    0xC0, 1, 0x03,
    0xC4, 1, 0x33,
    0xC5, 1, 0x80,
    0xC6, 1, 0x73,
    0xC7, 1, 0x00,
    0xC8, 2, 0x33, 0x33,
    0xC9, 1, 0x5B,
    0xCA, 1, 0x5A,
    0xCB, 1, 0x5D,
    0xCC, 1, 0x5C,
    0xCD, 2, 0x33, 0x33,
    0xCE, 1, 0x5F,
    0xCF, 1, 0x5E,
    0xD0, 1, 0x61,
    0xD1, 1, 0x60,
    0xB0, 4, 0x3A, 0x3A, 0x00, 0x00,
    0xB6, 1, 0x32,
    0xB7, 1, 0x80,
    0xB8, 1, 0x73,
    0xE0, 1, 0x00,
    0xE1, 2, 0x03, 0x0F,
    0xE2, 1, 0x04,
    0xE3, 1, 0x01,
    0xE4, 1, 0x0E,
    0xE5, 1, 0x01,
    0xE6, 1, 0x19,
    0xE7, 1, 0x10,
    0xE8, 1, 0x10,
    0xE9, 1, 0x21,
    0xEA, 1, 0x12,
    0xEB, 1, 0xD0,
    0xEC, 1, 0x04,
    0xED, 1, 0x07,
    0xEE, 1, 0x07,
    0xEF, 1, 0x09,
    0xF0, 1, 0xD0,
    0xF1, 1, 0x0E,
    0xF9, 1, 0x56,
    0xF2, 4, 0x26, 0x1B, 0x0B, 0x20,
    0xEC, 1, 0x04,
    0x35, 1, 0x00,
    0x44, 2, 0x00, 0x10,
    0x46, 1, 0x10,
    0xFF, 1, 0x00,
    0x3A, 1, 0x05,
    NV3007_SEQ_END
};

// Vendor sequence for the 2.79" 142x428 NV3007 module, from Arduino_GFX / LVGL
static const uint8_t NV3007_INIT_279[] = {
    0xFF, 1, 0xA5,
    0x9A, 1, 0x08,
    0x9B, 1, 0x08,
    0x9C, 1, 0xB0,
    0x9D, 1, 0x16,
    0x9E, 1, 0xC4,
    0x8F, 2, 0x55, 0x04,
    0x84, 1, 0x90,
    0x83, 1, 0x7B,
    0x85, 1, 0x33,
    0x60, 1, 0x00,
    0x70, 1, 0x00,
    0x61, 1, 0x02,
    0x71, 1, 0x02,
    0x62, 1, 0x04,
    0x72, 1, 0x04,
    0x6C, 1, 0x29,
    0x7C, 1, 0x29,
    0x6D, 1, 0x31,
    0x7D, 1, 0x31,
    0x6E, 1, 0x0F,
    0x7E, 1, 0x0F,
    0x66, 1, 0x21,
    0x76, 1, 0x21,
    0x68, 1, 0x3A,
    0x78, 1, 0x3A,
    0x63, 1, 0x07,
    0x73, 1, 0x07,
    0x64, 1, 0x05,
    0x74, 1, 0x05,
    0x65, 1, 0x02,
    0x75, 1, 0x02,
    0x67, 1, 0x23,
    0x77, 1, 0x23,
    0x69, 1, 0x08,
    0x79, 1, 0x08,
    0x6A, 1, 0x13,
    0x7A, 1, 0x13,
    0x6B, 1, 0x13,
    0x7B, 1, 0x13,
    0x6F, 1, 0x00,
    0x7F, 1, 0x00,
    0x50, 1, 0x00,
    0x52, 1, 0xD6,
    0x53, 1, 0x08,
    0x54, 1, 0x08,
    0x55, 1, 0x1E,
    0x56, 1, 0x1C,
    0xA0, 3, 0x2B, 0x24, 0x00,
    0xA1, 1, 0x87,
    0xA2, 1, 0x86,
    0xA5, 1, 0x00,
    0xA6, 1, 0x00,
    0xA7, 1, 0x00,
    0xA8, 1, 0x36,
    0xA9, 1, 0x7E,
    0xAA, 1, 0x7E,
    0xB9, 1, 0x85,
    0xBA, 1, 0x84,
    0xBB, 1, 0x83,
    0xBC, 1, 0x82,
    0xBD, 1, 0x81,
    0xBE, 1, 0x80,
    0xBF, 1, 0x01,
    0xC0, 1, 0x02,
    0xC1, 1, 0x00,
    0xC2, 1, 0x00,
    0xC3, 1, 0x00,
    0xC4, 1, 0x33,
    0xC5, 1, 0x7E,
    0xC6, 1, 0x7E,
    0xC8, 2, 0x33, 0x33,
    0xC9, 1, 0x68,
    0xCA, 1, 0x69,
    0xCB, 1, 0x6A,
    0xCC, 1, 0x6B,
    0xCD, 2, 0x33, 0x33,
    0xCE, 1, 0x6C,
    0xCF, 1, 0x6D,
    0xD0, 1, 0x6E,
    0xD1, 1, 0x6F,
    0xAB, 2, 0x03, 0x67,
    0xAC, 2, 0x03, 0x6B,
    0xAD, 2, 0x03, 0x68,
    0xAE, 2, 0x03, 0x6C,
    0xB3, 1, 0x00,
    0xB4, 1, 0x00,
    0xB5, 1, 0x00,
    0xB6, 1, 0x32,
    0xB7, 1, 0x7E,
    0xB8, 1, 0x7E,
    0xE0, 1, 0x00,
    0xE1, 2, 0x03, 0x0F,
    0xE2, 1, 0x04,
    0xE3, 1, 0x01,
    0xE4, 1, 0x0E,
    0xE5, 1, 0x01,
    0xE6, 1, 0x19,
    0xE7, 1, 0x10,
    0xE8, 1, 0x10,
    0xEA, 1, 0x12,
    0xEB, 1, 0xD0,
    0xEC, 1, 0x04,
    0xED, 1, 0x07,
    0xEE, 1, 0x07,
    0xEF, 1, 0x09,
    0xF0, 1, 0xD0,
    0xF1, 1, 0x0E,
    0xF9, 1, 0x17,
    0xF2, 4, 0x2C, 0x1B, 0x0B, 0x20,
    0xE9, 1, 0x29,
    0xEC, 1, 0x04,
    0x35, 1, 0x00,
    0x44, 2, 0x00, 0x10,
    0x46, 1, 0x10,
    0xFF, 1, 0x00,
    0x3A, 1, 0x05,
    NV3007_SEQ_END
};

GPGFX_NV3007::~GPGFX_NV3007() {
    if (_dmaChannel >= 0) {
        dma_channel_abort(_dmaChannel);
        dma_channel_unclaim(_dmaChannel);
        _dmaChannel = -1;
    }
}

void GPGFX_NV3007::init(GPGFX_DisplayTypeOptions options) {
    lastFrameValid = false;
    lastTransferUs = 0;
    _options = options;
    screenType = ScreenAlternatives::SCREEN_128x64_MAIN;

    const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
    const PeripheralOptions& peripheralOptions = Storage::getInstance().getPeripheralOptions();

    if (options.spi == nullptr) return;
    _spi = options.spi->getController();

    _pinCS = (displayOptions.spiDisplayBlock == 0) ? peripheralOptions.blockSPI0.cs : peripheralOptions.blockSPI1.cs;
    _pinDC = displayOptions.spiDisplayPinDC;
    _pinRST = displayOptions.spiDisplayPinReset;
    _pinBL = displayOptions.spiDisplayPinBacklight;

    if (_pinDC < 0) return;

    spi_set_baudrate(_spi, NV3007_SPI_BAUDRATE);
    spi_set_format(_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_init(_pinDC);
    gpio_set_dir(_pinDC, GPIO_OUT);
    gpio_put(_pinDC, 1);

    if (_pinCS >= 0) {
        gpio_init(_pinCS);
        gpio_set_dir(_pinCS, GPIO_OUT);
        gpio_put(_pinCS, 1);
    }

    if (_pinRST >= 0) {
        gpio_init(_pinRST);
        gpio_set_dir(_pinRST, GPIO_OUT);
        gpio_put(_pinRST, 1);
    }

    if (_pinBL >= 0) {
        // ~25kHz PWM so the brightness setting has no audible or visible flicker
        gpio_set_function(_pinBL, GPIO_FUNC_PWM);
        uint slice = pwm_gpio_to_slice_num(_pinBL);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_wrap(&config, 255);
        pwm_config_set_clkdiv(&config, (float)clock_get_hz(clk_sys) / (256.0f * 25000.0f));
        pwm_init(slice, &config, true);
        setBacklight(0);
    }

    if (_dmaChannel < 0) {
        _dmaChannel = dma_claim_unused_channel(false);
    }
    if (_dmaChannel >= 0) {
        dma_channel_config dmaConfig = dma_channel_get_default_config(_dmaChannel);
        channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_8);
        channel_config_set_read_increment(&dmaConfig, true);
        channel_config_set_write_increment(&dmaConfig, false);
        channel_config_set_dreq(&dmaConfig, spi_get_dreq(_spi, true));
        dma_channel_configure(_dmaChannel, &dmaConfig, &spi_get_hw(_spi)->dr, nullptr, 0, false);
    }

    _colorFg = toRGB565(displayOptions.spiDisplayForegroundColor);
    _colorBg = toRGB565(displayOptions.spiDisplayBackgroundColor);

    hardwareReset();

    sendInitSequence((displayOptions.spiDisplayInitSequence == SPI_DISPLAY_INIT_NV3007_279) ? NV3007_INIT_279 : NV3007_INIT_STANDARD);

    // Landscape: MV swaps the axes so the 428 pixel side runs left to right.
    // "Flip" mirrors the short (142) axis and "mirror" mirrors the long (428) axis.
    uint8_t madctl = MADCTL_MV | MADCTL_MX;
    if (_options.orientation & 0x01) madctl ^= MADCTL_MX;
    if (_options.orientation & 0x02) madctl ^= MADCTL_MY;
    sendCommand(CommandOps::MADCTL, &madctl, 1);

    // The 142 visible columns sit inside the 168 column GRAM, so mirroring that
    // axis moves the visible window to the opposite side of the GRAM.
    uint16_t offsetY = (uint16_t)std::max<int32_t>(0, displayOptions.spiDisplayOffsetY);
    _offsetX = (uint16_t)std::max<int32_t>(0, displayOptions.spiDisplayOffsetX);
    _offsetY = (madctl & MADCTL_MX) ? (uint16_t)std::max<int32_t>(0, (GRAM_SHORT_SIDE - PANEL_HEIGHT) - offsetY) : offsetY;

    sendCommand(_options.inverted ? CommandOps::INVON : CommandOps::INVOFF);

    configureView(displayOptions.spiDisplayScaleMode);

    // Clear GRAM before the panel is switched on to hide power-on garbage
    fillWindow(0, 0, PANEL_WIDTH - 1, PANEL_HEIGHT - 1, _colorBg);

    sendCommand(CommandOps::SLPOUT);
    sleep_ms(120);
    sendCommand(CommandOps::DISPON);
    sleep_ms(20);

    _isPowered = true;
    setBacklight(_options.contrast);

    clear();
    drawBuffer(NULL);
}

void GPGFX_NV3007::configureView(uint8_t scaleMode) {
    switch (scaleMode) {
        case SPI_DISPLAY_SCALE_FIT:
            _viewWidth = MAX_SCREEN_WIDTH * 2;
            _viewHeight = MAX_SCREEN_HEIGHT * 2;
            break;
        case SPI_DISPLAY_SCALE_FILL:
            _viewWidth = PANEL_WIDTH;
            _viewHeight = PANEL_HEIGHT;
            break;
        case SPI_DISPLAY_SCALE_STRETCH:
        default:
            _viewWidth = MAX_SCREEN_WIDTH * 3;
            _viewHeight = MAX_SCREEN_HEIGHT * 2;
            break;
    }
    _viewX = (PANEL_WIDTH - _viewWidth) / 2;
    _viewY = (PANEL_HEIGHT - _viewHeight) / 2;

    for (uint16_t x = 0; x < _viewWidth; x++) {
        _sourceX[x] = (uint8_t)(((uint32_t)x * MAX_SCREEN_WIDTH) / _viewWidth);
    }
    for (uint16_t y = 0; y < _viewHeight; y++) {
        _sourceY[y] = (uint8_t)(((uint32_t)y * MAX_SCREEN_HEIGHT) / _viewHeight);
    }
}

void GPGFX_NV3007::hardwareReset() {
    if (_pinRST >= 0) {
        gpio_put(_pinRST, 1);
        sleep_ms(10);
        gpio_put(_pinRST, 0);
        sleep_ms(10);
        gpio_put(_pinRST, 1);
        sleep_ms(120);
    } else {
        sendCommand(CommandOps::SWRESET);
        sleep_ms(120);
    }
}

void GPGFX_NV3007::setPower(bool isPowered) {
    if (_spi == nullptr || _pinDC < 0) return;

    if (isPowered) {
        sendCommand(CommandOps::SLPOUT);
        sleep_ms(120);
        sendCommand(CommandOps::DISPON);
        // Force a full redraw, GRAM content is not guaranteed after sleep
        lastFrameValid = false;
        setBacklight(_options.contrast);
    } else {
        setBacklight(0);
        sendCommand(CommandOps::DISPOFF);
        sendCommand(CommandOps::SLPIN);
        sleep_ms(5);
    }
    _isPowered = isPowered;
}

void GPGFX_NV3007::setBacklight(uint8_t level) {
    if (_pinBL < 0) return;
    // Level above the wrap value keeps the output high for full brightness
    pwm_set_gpio_level(_pinBL, (level == 0xFF) ? 256 : level);
}

void GPGFX_NV3007::drawBuffer(uint8_t* pBuffer) {
    if (_spi == nullptr || _pinDC < 0 || !_isPowered) return;
    if (pBuffer == NULL) pBuffer = frameBuffer;

    // Find the canvas rows that changed since the last frame that was sent
    int16_t firstRow = -1;
    int16_t lastRow = -1;
    if (lastFrameValid) {
        for (uint8_t page = 0; page < (MAX_SCREEN_HEIGHT / 8); page++) {
            uint8_t changed = 0;
            const uint8_t* current = &pBuffer[page * MAX_SCREEN_WIDTH];
            const uint8_t* previous = &lastFrameBuffer[page * MAX_SCREEN_WIDTH];
            for (uint16_t x = 0; x < MAX_SCREEN_WIDTH; x++) {
                changed |= current[x] ^ previous[x];
            }
            if (changed == 0) continue;
            int16_t pageFirst = page * 8 + __builtin_ctz(changed);
            int16_t pageLast = page * 8 + (31 - __builtin_clz(changed));
            if (firstRow < 0) firstRow = pageFirst;
            lastRow = pageLast;
        }
        if (firstRow < 0) return;
    } else {
        firstRow = 0;
        lastRow = MAX_SCREEN_HEIGHT - 1;
    }

    const uint32_t startedUs = time_us_32();
    if (pBuffer != lastFrameBuffer) {
        memcpy(lastFrameBuffer, pBuffer, MAX_SCREEN_SIZE);
    }

    if (!lastFrameValid) {
        // Repaint the border around the scaled view
        if (_viewY > 0) {
            fillWindow(0, 0, PANEL_WIDTH - 1, _viewY - 1, _colorBg);
            fillWindow(0, _viewY + _viewHeight, PANEL_WIDTH - 1, PANEL_HEIGHT - 1, _colorBg);
        }
        if (_viewX > 0) {
            fillWindow(0, _viewY, _viewX - 1, _viewY + _viewHeight - 1, _colorBg);
            fillWindow(_viewX + _viewWidth, _viewY, PANEL_WIDTH - 1, _viewY + _viewHeight - 1, _colorBg);
        }
    }

    // Map the changed canvas rows to the panel rows that sample them
    uint16_t firstPanelRow = 0;
    while (firstPanelRow < _viewHeight && _sourceY[firstPanelRow] < firstRow) firstPanelRow++;
    uint16_t lastPanelRow = _viewHeight - 1;
    while (lastPanelRow > firstPanelRow && _sourceY[lastPanelRow] > lastRow) lastPanelRow--;

    pushRows(firstPanelRow, lastPanelRow);

    lastFrameValid = true;
    lastTransferUs = time_us_32() - startedUs;
}

void GPGFX_NV3007::pushRows(uint16_t firstRow, uint16_t lastRow) {
    setWindow(_viewX, _viewY + firstRow, _viewX + _viewWidth - 1, _viewY + lastRow);

    if (_pinCS >= 0) gpio_put(_pinCS, 0);
    gpio_put(_pinDC, 0);
    uint8_t command = CommandOps::RAMWR;
    spi_write_blocking(_spi, &command, 1);
    gpio_put(_pinDC, 1);

    // Colors are stored byte-swapped so the 8-bit SPI stream is big endian RGB565
    const uint16_t fg = __builtin_bswap16(_colorFg);
    const uint16_t bg = __builtin_bswap16(_colorBg);

    uint8_t nextBuffer = 0;
    int16_t builtRow = -1;
    const uint16_t* line = nullptr;
    for (uint16_t row = firstRow; row <= lastRow; row++) {
        uint8_t sourceRow = _sourceY[row];
        if (sourceRow != builtRow) {
            // Build into the buffer that is not being transferred
            uint16_t* target = _lineBuffer[nextBuffer];
            const uint8_t* canvas = &lastFrameBuffer[(sourceRow / 8) * MAX_SCREEN_WIDTH];
            const uint8_t mask = 1 << (sourceRow % 8);
            for (uint16_t x = 0; x < _viewWidth; x++) {
                target[x] = (canvas[_sourceX[x]] & mask) ? fg : bg;
            }
            line = target;
            nextBuffer ^= 1;
            builtRow = sourceRow;
        }
        writePixels(line, _viewWidth);
    }

    waitForTransfer();
    if (_pinCS >= 0) gpio_put(_pinCS, 1);
}

void GPGFX_NV3007::fillWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    setWindow(x0, y0, x1, y1);

    if (_pinCS >= 0) gpio_put(_pinCS, 0);
    gpio_put(_pinDC, 0);
    uint8_t command = CommandOps::RAMWR;
    spi_write_blocking(_spi, &command, 1);
    gpio_put(_pinDC, 1);

    uint16_t width = x1 - x0 + 1;
    uint16_t* line = _lineBuffer[0];
    for (uint16_t x = 0; x < width; x++) line[x] = __builtin_bswap16(color);
    for (uint16_t y = y0; y <= y1; y++) {
        writePixels(line, width);
    }

    waitForTransfer();
    if (_pinCS >= 0) gpio_put(_pinCS, 1);
}

void GPGFX_NV3007::writePixels(const uint16_t* pixels, size_t count) {
    if (_dmaChannel >= 0) {
        dma_channel_wait_for_finish_blocking(_dmaChannel);
        dma_channel_transfer_from_buffer_now(_dmaChannel, pixels, count * sizeof(uint16_t));
    } else {
        spi_write_blocking(_spi, (const uint8_t*)pixels, count * sizeof(uint16_t));
    }
}

void GPGFX_NV3007::waitForTransfer() {
    if (_dmaChannel < 0) return;

    dma_channel_wait_for_finish_blocking(_dmaChannel);
    while (spi_is_busy(_spi)) tight_loop_contents();
    // The DMA only feeds TX, discard whatever was clocked into RX
    while (spi_is_readable(_spi)) (void)spi_get_hw(_spi)->dr;
    spi_get_hw(_spi)->icr = SPI_SSPICR_RORIC_BITS;
}

void GPGFX_NV3007::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += _offsetX;
    x1 += _offsetX;
    y0 += _offsetY;
    y1 += _offsetY;

    uint8_t columns[] = { (uint8_t)(x0 >> 8), (uint8_t)x0, (uint8_t)(x1 >> 8), (uint8_t)x1 };
    uint8_t rows[] = { (uint8_t)(y0 >> 8), (uint8_t)y0, (uint8_t)(y1 >> 8), (uint8_t)y1 };
    sendCommand(CommandOps::CASET, columns, sizeof(columns));
    sendCommand(CommandOps::RASET, rows, sizeof(rows));
}

void GPGFX_NV3007::sendCommand(uint8_t command) {
    sendCommand(command, nullptr, 0);
}

void GPGFX_NV3007::sendCommand(uint8_t command, const uint8_t* data, size_t length) {
    if (_pinCS >= 0) gpio_put(_pinCS, 0);
    gpio_put(_pinDC, 0);
    spi_write_blocking(_spi, &command, 1);
    if (length > 0) {
        gpio_put(_pinDC, 1);
        spi_write_blocking(_spi, data, length);
    }
    if (_pinCS >= 0) gpio_put(_pinCS, 1);
    gpio_put(_pinDC, 1);
}

void GPGFX_NV3007::sendInitSequence(const uint8_t* sequence) {
    while (true) {
        uint8_t command = *sequence++;
        uint8_t length = *sequence++;
        if (command == 0x00 && length == 0xFF) break;
        if (command == 0x00 && length == 0xFE) {
            sleep_ms(*sequence++);
            continue;
        }
        sendCommand(command, sequence, length);
        sequence += length;
    }
}

uint16_t GPGFX_NV3007::toRGB565(uint32_t rgb) {
    return (uint16_t)(((rgb >> 8) & 0xF800) | ((rgb >> 5) & 0x07E0) | ((rgb >> 3) & 0x001F));
}
