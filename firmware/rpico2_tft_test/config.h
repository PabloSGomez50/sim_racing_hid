#ifndef __CONFIGs_H__
#define __CONFIGs_H__

// Pin definitions for the LCD
#define TFT_SCLK        2
#define TFT_MOSI        3
#define TFT_DC          4
#define TFT_RST         5
#define TFT_CS          6
#define TFT_LED         1

#define TFT_WIDTH       320
#define TFT_HEIGHT      240
#define TFT_ROTATION    3
#define TFT_SPI_INSTANCE spi0

// Pin definitions for the SD card
// #define SD_SCLK         10
// #define SD_MOSI         11
// #define SD_MISO         12
// #define SD_CS           13

#define MARGIN 8
#define INDENT (MARGIN + 4)
#define LINE_HEIGHT 8
#define DELAY 5

// Colors are in 565 (FFFF) format. To convert from RGB888 to RGB565, use:
//   ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
#define BACKGROUND 0x1052
#define FOREGROUND 0x73BF

#endif