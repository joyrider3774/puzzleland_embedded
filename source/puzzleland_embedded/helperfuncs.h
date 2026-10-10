#ifndef helperfuncs_h
#define helperfuncs_h

#include <stdint.h>
//for PLATFORM_FAST_CODE, which marks the calls that run once for every pixel
#include "Platform.h"

void preloadImages(void);
uint8_t currentSkin(void);
void fillScreen(uint16_t color);
PLATFORM_FAST_CODE void fillRect(int x, int y, int w, int h, uint16_t color);
void drawRect(int x, int y, int w, int h, uint16_t color);
void printText(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size);
PLATFORM_FAST_CODE void drawImagePart(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth);
#if CARDIMAGES
//The same for a picture on the card, which can leave its transparent colour out. Nothing on the
//card is run length encoded, so this is where both of the drawing calls end up; see cardimages.h
PLATFORM_FAST_CODE void drawImagePart(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth, bool transparent);
#endif
void drawImage(int x, int y, int w, int h, const uint8_t* data);
PLATFORM_FAST_CODE void drawImageRLEPart(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth, int dataHeight, bool transparent);
void drawImageRLE(int x, int y, int w, int h, const uint8_t* data);
void drawImageRLETransparent(int x, int y, int w, int h, const uint8_t* data);
void drawBackgroundPart(int x, int y, int w, int h);
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t len, uint16_t color, uint16_t bg, uint8_t size);
#endif
