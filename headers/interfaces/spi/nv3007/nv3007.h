#ifndef _GPGFX_NV3007_H_
#define _GPGFX_NV3007_H_

#include "GPGFX_types.h"
#include "tiny_ssd1306.h"

#include "hardware/spi.h"

// SPI clock for the NV3007. The actual rate is the closest the SPI block can
// reach at or below this value (clk_peri / even divisor).
#ifndef NV3007_SPI_BAUDRATE
#define NV3007_SPI_BAUDRATE 40000000
#endif

// NV3007 TFT driver.
//
// The NV3007 drives 142x428 bar-type IPS panels over a 4-wire SPI bus
// (SCK/MOSI/CS/DC, plus optional RST and backlight). GP2040-CE's screens and
// layouts are authored for a 128x64 monochrome canvas, so this driver reuses
// the SSD1306 canvas and its drawing primitives unchanged, then upscales the
// canvas to the panel in landscape (428x142) with configurable foreground and
// background colors when the frame is pushed. Only the rows that changed since
// the previous frame are re-sent.
class GPGFX_NV3007 : public GPGFX_TinySSD1306 {
    public:
        GPGFX_NV3007() {
            _isSPI = true;
            _isI2C = false;
        }
        ~GPGFX_NV3007();

        void init(GPGFX_DisplayTypeOptions options);

        void setPower(bool isPowered);

        void drawBuffer(uint8_t *pBuffer);

        std::vector<uint8_t> getDeviceAddresses() const override {
            return {};
        }

        // Physical panel, in landscape orientation
        static const uint16_t PANEL_WIDTH = 428;
        static const uint16_t PANEL_HEIGHT = 142;
        // Controller GRAM is 168 columns wide; the 142 visible columns sit at an offset inside it
        static const uint16_t GRAM_SHORT_SIDE = 168;
    private:
        typedef enum {
            SWRESET = 0x01,
            SLPIN = 0x10,
            SLPOUT = 0x11,
            INVOFF = 0x20,
            INVON = 0x21,
            DISPOFF = 0x28,
            DISPON = 0x29,
            CASET = 0x2A,
            RASET = 0x2B,
            RAMWR = 0x2C,
            MADCTL = 0x36,
            COLMOD = 0x3A,
        } CommandOps;

        static const uint8_t MADCTL_MY = 0x80;
        static const uint8_t MADCTL_MX = 0x40;
        static const uint8_t MADCTL_MV = 0x20;

        spi_inst_t* _spi = nullptr;
        int8_t _pinCS = -1;
        int8_t _pinDC = -1;
        int8_t _pinRST = -1;
        int8_t _pinBL = -1;
        int _dmaChannel = -1;

        uint16_t _offsetX = 0;
        uint16_t _offsetY = 0;

        uint16_t _colorFg = 0xFFFF;
        uint16_t _colorBg = 0x0000;

        // Area of the panel the canvas is scaled into
        uint16_t _viewX = 0;
        uint16_t _viewY = 0;
        uint16_t _viewWidth = PANEL_WIDTH;
        uint16_t _viewHeight = PANEL_HEIGHT;

        // Canvas column/row sampled by each panel column/row of the view
        uint8_t _sourceX[PANEL_WIDTH];
        uint8_t _sourceY[PANEL_HEIGHT];

        // RGB565 line buffers, byte-swapped for the wire, double buffered for DMA
        uint16_t _lineBuffer[2][PANEL_WIDTH];

        void hardwareReset();
        void sendCommand(uint8_t command);
        void sendCommand(uint8_t command, const uint8_t* data, size_t length);
        void sendInitSequence(const uint8_t* sequence);
        void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
        void writePixels(const uint16_t* pixels, size_t count);
        void waitForTransfer();
        void fillWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
        void pushRows(uint16_t firstRow, uint16_t lastRow);
        void setBacklight(uint8_t level);
        void configureView(uint8_t scaleMode);

        static uint16_t toRGB565(uint32_t rgb);
};

#endif
