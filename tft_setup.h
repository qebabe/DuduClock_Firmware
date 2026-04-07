#pragma once

// Use a sketch-local TFT_eSPI setup so this repo does not depend on the
// globally selected User_Setup.h inside the Arduino library folder.
#define USER_SETUP_LOADED

#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ESP32-S3 works more reliably with the display on HSPI in this project.
#define USE_HSPI_PORT

#define TFT_MISO -1
#define TFT_MOSI 47
#define TFT_SCLK 21
#define TFT_CS   41
#define TFT_DC   40
#define TFT_RST  45
#define TFT_BL   42
#define TFT_BACKLIGHT_ON HIGH

#define TFT_RGB_ORDER TFT_BGR
#define SPI_FREQUENCY 10000000

#define SMOOTH_FONT
