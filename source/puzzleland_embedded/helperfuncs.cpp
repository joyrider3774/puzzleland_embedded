#include <stdint.h>
#include "commonvars.h"
#include "bandrender.h"
#include "helperfuncs.h"
//the one bit pictures of the black & white skin, which the band renderer reads too
#include "onebitimage.h"

#if SCREENBUFFER == 0
//Where every row of a whole screen background starts in its encoded form, and how many pixels of
//the control there belong to the rows above it.
//
//Without a screen buffer only the places that changed are drawn again, each with its own call, and
//every one of those would otherwise find its first row by walking the image from the start: the
//piece at the bottom of the between stage screen is over 1500 controls in, and a frame that paints
//eight or nine places walks several thousand of them for nothing. It is written down once per
//background instead. Only a whole screen one is worth it, sprites are small and start near the top
static uint16_t rleRowOffset[WINDOW_HEIGHT];
static uint8_t rleRowUsed[WINDOW_HEIGHT];
static const uint8_t* rleIndexed = NULL;

//an encoded image is at most three bytes per pixel (a run of one every time), so the offsets of a
//whole screen stay inside a uint16_t, and a control covers at most WINDOW_WIDTH pixels
static_assert(WINDOW_WIDTH * WINDOW_HEIGHT * 3 <= 65535, "the row offsets do not fit in a uint16_t");
static_assert(WINDOW_WIDTH <= 255, "the pixels of a control do not fit in a uint8_t");

static void IndexRLERows(const uint8_t* image)
{
    const uint8_t* data = image;
    uint32_t pixel = 0;
    int16_t row = 0;
    while (row < WINDOW_HEIGHT)
    {
        const uint8_t control = PLATFORM_READ_BYTE(data);
        const uint32_t count = (uint32_t)(control & 0x7F) + 1;
        //every row that starts inside this control
        while ((row < WINDOW_HEIGHT) && ((uint32_t)row * WINDOW_WIDTH < pixel + count))
        {
            rleRowOffset[row] = (uint16_t)(data - image);
            rleRowUsed[row] = (uint8_t)((uint32_t)row * WINDOW_WIDTH - pixel);
            row++;
        }
        pixel += count;
        data += (control & 0x80) ? 3 : 1 + count * 2;
    }
    rleIndexed = image;
}
#endif

//A row of an image on its way to the display. Where flash is plain memory an evenly placed
//row is handed over where it lies, otherwise it is copied into the scratch row first. A 16
//bit read needs an even address, a core like the Cortex-M0+ faults on an odd one
static inline const uint16_t* ImageRow(const void* src, uint16_t* scratch, int count)
{
#if PLATFORM_DIRECT_FLASH
    if (((uintptr_t)src & 1) == 0)
        return (const uint16_t*)src;
#endif
    PLATFORM_READ_BYTES((uint8_t*)scratch, src, count * sizeof(uint16_t));
    return scratch;
}

//only the skin FORCESKIN picks is part of the build (a 1 bpp buffer takes the black & white one
//unless the build asks for another)
#if FORCESKIN == skinDefault
#include "images/default/betweenstage_RLE565.h"
#include "images/default/betweenstagelevel1_RLE565.h"
#include "images/default/blockactiveimage_RGB565_LE.h"
#include "images/default/blockimage_RGB565_LE.h"
#include "images/default/border1_RGB565_LE.h"
#include "images/default/border2_RGB565_LE.h"
#include "images/default/border3_RGB565_LE.h"
#include "images/default/border4_RGB565_LE.h"
#include "images/default/border5_RGB565_LE.h"
#include "images/default/border6_RGB565_LE.h"
#include "images/default/border7_RGB565_LE.h"
#include "images/default/bridge_RLE565.h"
#include "images/default/hand_RLE565.h"
#include "images/default/intro_RLE565.h"
#include "images/default/oldman_RLE565.h"
#include "images/default/optionsselect_RLE565.h"
#include "images/default/paper_RLE565.h"
#include "images/default/roombackground_RLE565.h"
#include "images/default/ryf_cloud_RLE565.h"
#include "images/default/ryf_fairy_RLE565.h"
#include "images/default/ryf_player_RLE565.h"
#include "images/default/ryf_shadow_RLE565.h"
#include "images/default/ryf_smallcloud_RLE565.h"
#include "images/default/select_RLE565.h"
#include "images/default/spaceship_RLE565.h"
#include "images/default/stageblock1_RLE565.h"
#include "images/default/stageblock2_RLE565.h"
#include "images/default/stageblock3_RLE565.h"
#include "images/default/stageblock4_RLE565.h"
#include "images/default/stageblock5_RLE565.h"
#include "images/default/stageblock6_RLE565.h"
#include "images/default/stageblock7_RLE565.h"
#include "images/default/stageblock8_RLE565.h"
#include "images/default/stageblock9_RLE565.h"
#include "images/default/stageblock10_RLE565.h"
#include "images/default/stageclearkader_RLE565.h"
#include "images/default/title_RLE565.h"
#include "images/default/titleselector_RLE565.h"
#endif

#if FORCESKIN == skinBlackWhite
#include "images/black_white/betweenstage_RLE565.h"
#include "images/black_white/betweenstagelevel1_RLE565.h"
#include "images/black_white/blockactiveimage_RGB565_LE.h"
#include "images/black_white/blockimage_RGB565_LE.h"
#include "images/black_white/border1_RGB565_LE.h"
#include "images/black_white/border2_RGB565_LE.h"
#include "images/black_white/border3_RGB565_LE.h"
#include "images/black_white/border4_RGB565_LE.h"
#include "images/black_white/border5_RGB565_LE.h"
#include "images/black_white/border6_RGB565_LE.h"
#include "images/black_white/border7_RGB565_LE.h"
#include "images/black_white/bridge_RLE565.h"
#include "images/black_white/hand_RLE565.h"
#include "images/black_white/intro_RLE565.h"
#include "images/black_white/oldman_RLE565.h"
#include "images/black_white/optionsselect_RLE565.h"
#include "images/black_white/paper_RLE565.h"
#include "images/black_white/roombackground_RLE565.h"
#include "images/black_white/ryf_cloud_RLE565.h"
#include "images/black_white/ryf_fairy_RLE565.h"
#include "images/black_white/ryf_player_RLE565.h"
#include "images/black_white/ryf_shadow_RLE565.h"
#include "images/black_white/ryf_smallcloud_RLE565.h"
#include "images/black_white/select_RLE565.h"
#include "images/black_white/spaceship_RLE565.h"
#include "images/black_white/stageblock1_RLE565.h"
#include "images/black_white/stageblock2_RLE565.h"
#include "images/black_white/stageblock3_RLE565.h"
#include "images/black_white/stageblock4_RLE565.h"
#include "images/black_white/stageblock5_RLE565.h"
#include "images/black_white/stageblock6_RLE565.h"
#include "images/black_white/stageblock7_RLE565.h"
#include "images/black_white/stageblock8_RLE565.h"
#include "images/black_white/stageblock9_RLE565.h"
#include "images/black_white/stageblock10_RLE565.h"
#include "images/black_white/stageclearkader_RLE565.h"
#include "images/black_white/title_RLE565.h"
#include "images/black_white/titleselector_RLE565.h"
#endif

//the game draws the images with the sizes in defines.h, a skin has to keep to them
#if FORCESKIN == skinDefault
#define SKIN_IMAGE(name) default_##name
#else
#define SKIN_IMAGE(name) black_white_##name
#endif
static_assert((SKIN_IMAGE(betweenstage_width) == fullScreenWidth) && (SKIN_IMAGE(betweenstage_height) == fullScreenHeight) &&
			  (SKIN_IMAGE(betweenstagelevel1_width) == fullScreenWidth) && (SKIN_IMAGE(betweenstagelevel1_height) == fullScreenHeight) &&
			  (SKIN_IMAGE(intro_width) == fullScreenWidth) && (SKIN_IMAGE(intro_height) == fullScreenHeight) &&
			  (SKIN_IMAGE(oldman_width) == fullScreenWidth) && (SKIN_IMAGE(oldman_height) == fullScreenHeight) &&
			  (SKIN_IMAGE(paper_width) == fullScreenWidth) && (SKIN_IMAGE(paper_height) == fullScreenHeight) &&
			  (SKIN_IMAGE(roombackground_width) == fullScreenWidth) && (SKIN_IMAGE(roombackground_height) == fullScreenHeight) &&
			  (SKIN_IMAGE(title_width) == fullScreenWidth) && (SKIN_IMAGE(title_height) == fullScreenHeight),
			  "a full screen image of the skin is not 128x128");
static_assert((SKIN_IMAGE(blockactiveimage_width) == BlockWidth) && (SKIN_IMAGE(blockactiveimage_height) == BlockHeight) &&
			  (SKIN_IMAGE(blockimage_width) == BlockWidth) && (SKIN_IMAGE(blockimage_height) == BlockHeight) &&
              (SKIN_IMAGE(border1_width) == BlockWidth) && (SKIN_IMAGE(border1_height) == BlockHeight) &&
              (SKIN_IMAGE(border2_width) == BlockWidth) && (SKIN_IMAGE(border2_height) == BlockHeight) &&
              (SKIN_IMAGE(border3_width) == BlockWidth) && (SKIN_IMAGE(border3_height) == BlockHeight) &&
              (SKIN_IMAGE(border4_width) == BlockWidth) && (SKIN_IMAGE(border4_height) == BlockHeight) &&
              (SKIN_IMAGE(border5_width) == BlockWidth) && (SKIN_IMAGE(border5_height) == BlockHeight) &&
              (SKIN_IMAGE(border6_width) == BlockWidth) && (SKIN_IMAGE(border6_height) == BlockHeight) &&
              (SKIN_IMAGE(border7_width) == BlockWidth) && (SKIN_IMAGE(border7_height) == BlockHeight) &&
			  "a border or block image of the skin is not 12x12");
static_assert((SKIN_IMAGE(stageblock1_width) == stageBlock1Width) && (SKIN_IMAGE(stageblock1_height) == stageBlock1Height) &&
              (SKIN_IMAGE(stageblock2_width) == stageBlock2Width) && (SKIN_IMAGE(stageblock2_height) == stageBlock2Height) &&
              (SKIN_IMAGE(stageblock3_width) == stageBlock3Width) && (SKIN_IMAGE(stageblock3_height) == stageBlock3Height) &&
              (SKIN_IMAGE(stageblock4_width) == stageBlock4Width) && (SKIN_IMAGE(stageblock4_height) == stageBlock4Height) &&
              (SKIN_IMAGE(stageblock5_width) == stageBlock5Width) && (SKIN_IMAGE(stageblock5_height) == stageBlock5Height) &&
              (SKIN_IMAGE(stageblock6_width) == stageBlock6Width) && (SKIN_IMAGE(stageblock6_height) == stageBlock6Height) &&
              (SKIN_IMAGE(stageblock7_width) == stageBlock7Width) && (SKIN_IMAGE(stageblock7_height) == stageBlock7Height) &&
              (SKIN_IMAGE(stageblock8_width) == stageBlock8Width) && (SKIN_IMAGE(stageblock8_height) == stageBlock8Height) &&
              (SKIN_IMAGE(stageblock9_width) == stageBlock9Width) && (SKIN_IMAGE(stageblock9_height) == stageBlock9Height) &&
              (SKIN_IMAGE(stageblock10_width) == stageBlock10Width) && (SKIN_IMAGE(stageblock10_height) == stageBlock10Height),
			  "a stageblock image of the skin does not have the size in defines.h");
static_assert((SKIN_IMAGE(bridge_width) == bridgeWidth) && (SKIN_IMAGE(bridge_height) == bridgeHeight) &&
              (SKIN_IMAGE(hand_width) == handWidth) && (SKIN_IMAGE(hand_height) == handHeight) &&
              (SKIN_IMAGE(ryf_cloud_width) == ryfCloudWidth) && (SKIN_IMAGE(ryf_cloud_height) == ryfCloudHeight) &&
              (SKIN_IMAGE(ryf_shadow_width) == ryfShadowWidth) && (SKIN_IMAGE(ryf_shadow_height) == ryfShadowHeight) &&
              (SKIN_IMAGE(ryf_smallcloud_width) == ryfSmallCloudWidth) && (SKIN_IMAGE(ryf_smallcloud_height) == ryfSmallCloudHeight) &&
              (SKIN_IMAGE(select_width) == selectWidth) && (SKIN_IMAGE(select_height) == selectHeight) &&
              (SKIN_IMAGE(titleselector_width) == titleSelectorWidth) && (SKIN_IMAGE(titleselector_height) == titleSelectorHeight) &&
              (SKIN_IMAGE(spaceship_width) == spaceshipWidth) && (SKIN_IMAGE(spaceship_height) == spaceshipHeight) &&
              (SKIN_IMAGE(stageclearkader_width) == stageClearKaderWidth) && (SKIN_IMAGE(stageclearkader_height) == stageClearKaderHeight),
			  "an other image of the skin does not have the size in defines.h");
static_assert((SKIN_IMAGE(ryf_fairy_width) == ryfFairyWidth) && (SKIN_IMAGE(ryf_fairy_height) == ryfFairyHeight * 2),
			  "the fairy image of the skin is not 2 tiles of 13 x 16");              
static_assert((SKIN_IMAGE(ryf_player_width) == ryfPlayerWidth) && (SKIN_IMAGE(ryf_player_height) == ryfPlayerHeight * 19),
			  "the player image of the skin is not 19 tiles of 14 x 14");

//the skin in use, the one FORCESKIN builds in
uint8_t currentSkin(void)
{
    return FORCESKIN;
}

void preloadImages(void)
{
    switch(currentSkin())
    {
#if FORCESKIN == skinDefault
        case skinDefault:
            ColorBackground = SCREEN.color565(255,255,255);
            ColorForeground = SCREEN.color565(0,0,0);
            ColorText = SCREEN.color565(0,0,0);
            ColorPaper = SCREEN.color565(255,255,255);
            break;
#endif
#if FORCESKIN == skinBlackWhite
        case skinBlackWhite:
            ColorBackground = SCREEN.color565(0,0,0);
            ColorForeground = SCREEN.color565(255,255,255);
            ColorText = SCREEN.color565(0,0,0);
            //the paper is a white sheet in this skin too, and the text on it stays black
            ColorPaper = SCREEN.color565(255,255,255);
            break;
#endif
    }
    BorderImages[0] = SKIN_IMAGE(border1_data);
    BorderImages[1] = SKIN_IMAGE(border2_data);
    BorderImages[2] = SKIN_IMAGE(border3_data);
    BorderImages[3] = SKIN_IMAGE(border4_data);
    BorderImages[4] = SKIN_IMAGE(border5_data);
    BorderImages[5] = SKIN_IMAGE(border6_data);
    BorderImages[6] = SKIN_IMAGE(border7_data);

    ImgStageBlock1 = SKIN_IMAGE(stageblock1_rle); 
    ImgStageBlock2 = SKIN_IMAGE(stageblock2_rle);
    ImgStageBlock3 = SKIN_IMAGE(stageblock3_rle); 
    ImgStageBlock4 = SKIN_IMAGE(stageblock4_rle);
    ImgStageBlock5 = SKIN_IMAGE(stageblock5_rle); 
    ImgStageBlock6 = SKIN_IMAGE(stageblock6_rle);
    ImgStageBlock7 = SKIN_IMAGE(stageblock7_rle);
    ImgStageBlock8 = SKIN_IMAGE(stageblock8_rle);
    ImgStageBlock9 = SKIN_IMAGE(stageblock9_rle); 
    ImgStageBlock10 = SKIN_IMAGE(stageblock10_rle); 
    
    ImgRyfCloud = SKIN_IMAGE(ryf_cloud_rle);
    ImgRyfSmallCloud = SKIN_IMAGE(ryf_smallcloud_rle); 
    ImgRyfFairy = SKIN_IMAGE(ryf_fairy_rle);
    ImgSelect = SKIN_IMAGE(select_rle); 
    ImgHand = SKIN_IMAGE(hand_rle);
    ImgOptionSelect = SKIN_IMAGE(optionsselect_rle);
    ImgShadow = SKIN_IMAGE(ryf_shadow_rle); 
    ImgPlayer = SKIN_IMAGE(ryf_player_rle); 
    ImgSpaceship = SKIN_IMAGE(spaceship_rle); 
    ImgTitleSelector = SKIN_IMAGE(titleselector_rle);
#if PAPERBACKGROUND
    ImgPaper = SKIN_IMAGE(paper_rle);
#endif
    ImgRoomBackground = SKIN_IMAGE(roombackground_rle);
    ImgBlockActiveImage = SKIN_IMAGE(blockactiveimage_data);
    ImgBlockImage = SKIN_IMAGE(blockimage_data);
#if INTROSCREEN
    ImgIntro = SKIN_IMAGE(intro_rle);
#endif
    ImgBetweenStage = SKIN_IMAGE(betweenstage_rle);
#if BETWEENSTAGEFIRSTPICTURE
    ImgBetweenStageLevel1 = SKIN_IMAGE(betweenstagelevel1_rle);
#endif
    ImgBridge = SKIN_IMAGE(bridge_rle);
    ImgOldMan = SKIN_IMAGE(oldman_rle);
    ImgStageClearKader = SKIN_IMAGE(stageclearkader_rle); 
    ImgTitle = SKIN_IMAGE(title_rle);
}

void fillScreen(uint16_t color)
{
    GFX.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, color);
}

void fillRect(int x, int y, int w, int h, uint16_t color)
{
#if SCREENBUFFER == 0
    //into the strip being put together, when there is one. See bandrender.h
    if (BandRender_Drawing())
    {
        BandRender_Fill(x, y, w, h, color);
        return;
    }
#endif
    GFX.fillRect(x, y, w, h, color);
}

void drawRect(int x, int y, int w, int h, uint16_t color)
{
    GFX.drawRect(x, y, w, h, color);
}

//multi line text in the 6x8 GLCD font, a cell of 6x8 per character with the background only
//painted when it differs from the text colour, 6 pixels per char and 9 per line
void printText(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size)
{
	int16_t cursorX = x;
	int16_t cursorY = y;
	if (!str)
		return;
#if LOVYANGFX
	//LovyanGFX's drawChar that takes the colours hands them to the font the other way
	//round, set them as the text colour instead. Its default font is the same 6x8 GLCD
	//font and a background equal to the text colour is left out here as well
	GFX.setTextColor(color, bg);
	GFX.setTextSize(size);
#endif
#if SCREENBUFFER == 0
	//Straight to the display every character would be a write transaction of its own, and
	//the chip select sits on the I/O expander: that is I2C traffic per character. One
	//transaction for the whole text instead. Into a buffer nothing is sent, so nothing to do
	SCREEN.startWrite();
#endif
	while (*str)
	{
		if (*str == '\n')
		{
			cursorY += 9 * size;
			cursorX = x;
			str++;
			continue;
		}
#if LOVYANGFX
		GFX.drawChar((uint8_t)*str, cursorX, cursorY);
#else
		GFX.drawChar(cursorX, cursorY, *str, color, bg, size);
#endif
		cursorX += 6 * size;
		str++;
	}
#if SCREENBUFFER == 0
	SCREEN.endWrite();
#endif
}

//Draws the w x h part at sx,sy of a raw RGB565 little endian image that is dataWidth pixels
//wide, at x,y on the screen and clipped to it. Every visible row comes out of flash in one
//copy: LovyanGFX reads image data through plain pointers, but PROGMEM on the ESP8266 is flash
//that only takes 32 bit reads, so the rows are read here with PLATFORM_READ_BYTES
void drawImagePart(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth)
{
    if (!data)
        return;
#if ONEBITIMAGES
  #if SCREENBUFFER == 0
    //into the strip being put together, when there is one. See bandrender.h
    if (BandRender_Drawing())
    {
        BandRender_ImageOneBit(x, y, sx, sy, w, h, data, false);
        return;
    }
  #endif
    //the skin's pictures carry their own width, the one passed in is the RGB565 path's
    (void)dataWidth;
    drawImageOneBitPart(x, y, sx, sy, w, h, data, false);
}
#else
#if SCREENBUFFER == 0
    //into the strip being put together, when there is one. See bandrender.h
    if (BandRender_Drawing())
    {
        BandRender_Image(x, y, sx, sy, w, h, data, dataWidth);
        return;
    }
#endif
    //the columns and rows of the part that are on screen
    const int c0 = (x < 0) ? -x : 0;
    const int c1 = (x + w > WINDOW_WIDTH) ? WINDOW_WIDTH - x : w;
    const int r0 = (y < 0) ? -y : 0;
    const int r1 = (y + h > WINDOW_HEIGHT) ? WINDOW_HEIGHT - y : h;
    if ((c0 >= c1) || (r0 >= r1))
        return;
    const int cols = c1 - c0;
    const int dx = x + c0;
    //little endian RGB565 like every device, so the bytes can be copied straight into it
    uint16_t row[WINDOW_WIDTH];
#if SCREENBUFFER
    void* buffer = SCREENBUFFER_PIXELS();
    if (!buffer)
        return;
    for (int r = r0; r < r1; r++)
    {
        const int dy = y + r;
        PLATFORM_READ_BYTES((uint8_t*)row, data + ((sy + r) * dataWidth + sx + c0) * sizeof(uint16_t), cols * sizeof(uint16_t));
  #if SCREENBUFFER == 16
        uint16_t* d = &((uint16_t*)buffer)[dy * WINDOW_WIDTH + dx];
        //a 16 bpp sprite keeps its pixels byte swapped
        for (int c = 0; c < cols; c++)
            d[c] = (uint16_t)((row[c] >> 8) | (row[c] << 8));
  #elif SCREENBUFFER == 8
        uint8_t* d = &((uint8_t*)buffer)[dy * WINDOW_WIDTH + dx];
        //RGB332, the same conversion SetBufferPixel does
        for (int c = 0; c < cols; c++)
            d[c] = ToBuffer332(row[c], (int16_t)(dx + c), (int16_t)dy);
  #else
        for (int c = 0; c < cols; c++)
            SetBufferBit((uint8_t*)buffer, dx + c, dy, row[c]);
  #endif
    }
#else
    //straight to the display. The chip select sits on the I/O expander, every write
    //transaction costs I2C traffic, so all the rows go out in one
    SCREEN.startWrite();
  #if LOVYANGFX
    //one window for the whole part, filled a row at a time
    SCREEN.setAddrWindow(dx, y + r0, cols, r1 - r0);
  #endif
    for (int r = r0; r < r1; r++)
    {
        const uint16_t* prow = ImageRow(data + ((sy + r) * dataWidth + sx + c0) * sizeof(uint16_t), row, cols);
  #if LOVYANGFX
        //true: the values are plain RGB565, the library puts them in display order
        SCREEN.writePixels(prow, cols, true);
  #else
        GFX.pushImage(dx, y + r, cols, 1, prow);
  #endif
    }
    SCREEN.endWrite();
#endif
}
#endif

void drawImage(int x, int y, int w, int h, const uint8_t* data)
{
    drawImagePart(x, y, 0, 0, w, h, data, w);
}

//Draws the w x h part at sx,sy of a run length encoded RGB565 image made by tools/png2rle565.py
//that is dataWidth x dataHeight, at x,y on the screen and clipped to it. A control byte with the
//top bit set is a run of (c & 0x7F) + 1 times the pixel after it, otherwise c + 1 literal pixels
//follow. The image can only be read from its start, so it is decoded up to the last row of the
//part and only the pixels of the part are drawn. With transparent set its COLOR_TRANSPARENT pixels
//are skipped. The data is read with PLATFORM_READ_BYTE and PLATFORM_READ_BYTES: LovyanGFX reads
//image data through plain pointers, but PROGMEM on the ESP8266 is flash that only takes 32 bit reads
void drawImageRLEPart(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth, int dataHeight, bool transparent)
{
    if (!data || (w <= 0) || (h <= 0))
        return;
#if ONEBITIMAGES
  #if SCREENBUFFER == 0
    //into the strip being put together, when there is one. See bandrender.h
    if (BandRender_Drawing())
    {
        BandRender_ImageOneBit(x, y, sx, sy, w, h, data, transparent);
        return;
    }
  #endif
    //the skin's pictures carry their own size, the ones passed in are the RGB565 path's
    (void)dataWidth;
    (void)dataHeight;
    drawImageOneBitPart(x, y, sx, sy, w, h, data, transparent);
}
#else
#if SCREENBUFFER == 0
    //into the strip being put together, when there is one. See bandrender.h
    if (BandRender_Drawing())
    {
        BandRender_ImageRLE(x, y, sx, sy, w, h, data, dataWidth, dataHeight, transparent);
        return;
    }
#endif
    //where the image's top left corner lands on the screen
    const int ox = x - sx;
    const int oy = y - sy;
    //the columns and rows of the image that are drawn: the part, cut to the image and the screen
    int c0 = sx, c1 = sx + w, r0 = sy, r1 = sy + h;
    if (c0 < 0) c0 = 0;
    if (r0 < 0) r0 = 0;
    if (c1 > dataWidth) c1 = dataWidth;
    if (r1 > dataHeight) r1 = dataHeight;
    if (c0 < -ox) c0 = -ox;
    if (r0 < -oy) r0 = -oy;
    if (c1 > WINDOW_WIDTH - ox) c1 = WINDOW_WIDTH - ox;
    if (r1 > WINDOW_HEIGHT - oy) r1 = WINDOW_HEIGHT - oy;
    if ((c0 >= c1) || (r0 >= r1))
        return;
#if SCREENBUFFER
    void* dst = SCREENBUFFER_PIXELS();
    if (!dst)
        return;
#else
    //the chip select sits on the I/O expander, so the whole part goes out in one transaction
    //and, without transparency, one window. LovyanGFX's pushBlock and pushPixels open one of
    //their own per call, there the write variants are used inside this one
    SCREEN.startWrite();
    if (!transparent)
        SCREEN.setAddrWindow(ox + c0, oy + r0, c1 - c0, r1 - r0);
#endif
    //a control covers at most 128 pixels
    uint16_t pixels[128];
    uint32_t left = (uint32_t)dataWidth * dataHeight;
    int cx = 0, cy = 0;
#if SCREENBUFFER == 0
    //a whole screen background starts at the row that is wanted instead of at its own start
    if ((dataWidth == WINDOW_WIDTH) && (dataHeight == WINDOW_HEIGHT) && (r0 > 0))
    {
        if (rleIndexed != data)
            IndexRLERows(data);
        //the control the row starts in, which the rows above it have already taken part of
        const uint32_t start = (uint32_t)r0 * WINDOW_WIDTH - rleRowUsed[r0];
        data += rleRowOffset[r0];
        left -= start;
        cx = (int)(start % (uint32_t)dataWidth);
        cy = (int)(start / (uint32_t)dataWidth);
    }
#endif
    //decoding stops after the last row of the part
    while ((left > 0) && (cy < r1))
    {
        uint8_t control = PLATFORM_READ_BYTE(data++);
        uint16_t count = (control & 0x7F) + 1;
        if (count > left)
            count = (uint16_t)left;
        const bool run = (control & 0x80) != 0;
        uint16_t color = 0;
        if (run)
        {
            color = PLATFORM_READ_BYTE(data) | (PLATFORM_READ_BYTE(data + 1) << 8);
            data += 2;
        }
        else
        {
            //pixels that all lie in rows above the part are only skipped, a control that
            //reaches into the part is read
            if ((uint32_t)(cy * dataWidth + cx + count - 1) / dataWidth >= (uint32_t)r0)
                PLATFORM_READ_BYTES((uint8_t*)pixels, data, count * sizeof(uint16_t));
            data += count * sizeof(uint16_t);
        }
        left -= count;
        //the pixels of the control a row at a time, only the part is drawn
        for (uint16_t done = 0; done < count; )
        {
            int n = dataWidth - cx;
            if (n > count - done)
                n = count - done;
            if ((cy >= r0) && (cy < r1))
            {
                const int a = (cx > c0) ? cx : c0;
                const int e = (cx + n < c1) ? cx + n : c1;
                if (a < e)
                {
                    const uint16_t* src = &pixels[done + (a - cx)];
#if SCREENBUFFER
                    const int sy2 = oy + cy;
                    for (int c = a; c < e; c++)
                    {
                        const uint16_t pixel = run ? color : src[c - a];
                        if (!transparent || (pixel != COLOR_TRANSPARENT))
                            SetBufferPixel(dst, ox + c, sy2, pixel);
                    }
#else
                    //without transparency the pixels fill the one window in order, with it every
                    //run of opaque pixels goes out in a window of its own
                    int c = a;
                    while (c < e)
                    {
                        int start = c;
                        if (transparent)
                        {
                            while ((c < e) && ((run ? color : src[c - a]) == COLOR_TRANSPARENT))
                                c++;
                            start = c;
                            while ((c < e) && ((run ? color : src[c - a]) != COLOR_TRANSPARENT))
                                c++;
                            if (start == c)
                                continue;
                            SCREEN.setAddrWindow(ox + start, oy + cy, c - start, 1);
                        }
                        else
                            c = e;
  #if LOVYANGFX
                        //a uint16_t colour is taken as plain RGB565, true: so are the pixels
                        if (run)
                            SCREEN.writeColor(color, c - start);
                        else
                            SCREEN.writePixels(&src[start - a], c - start, true);
  #else
                        if (run)
                            SCREEN.pushBlock(color, c - start);
                        else
                            SCREEN.pushPixels((uint16_t*)&src[start - a], c - start);
  #endif
                    }
#endif
                }
            }
            done += n;
            cx += n;
            if (cx == dataWidth)
            {
                cx = 0;
                cy++;
            }
        }
    }
#if !SCREENBUFFER
    SCREEN.endWrite();
#endif
}
#endif

void drawImageRLE(int x, int y, int w, int h, const uint8_t* data)
{
    drawImageRLEPart(x, y, 0, 0, w, h, data, w, h, false);
}

void drawImageRLETransparent(int x, int y, int w, int h, const uint8_t* data)
{
    drawImageRLEPart(x, y, 0, 0, w, h, data, w, h, true);
}

//multi line text straight to the screen. tft.drawChar already matches what the
//framebuffer version did per character, a 6x8 cell with the background only
//painted when it differs from the text colour, so only the line breaks are
//handled here. Advances match the old code, 6 pixels per char and 9 per line
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t len, uint16_t color, uint16_t bg, uint8_t size)
{
	int16_t cursorX = x;
	int16_t cursorY = y;
	if (!str)
		return;
#if LOVYANGFX
	//LovyanGFX's drawChar that takes the colours hands them to the font the other way
	//round, set them as the text colour instead. Its default font is the same 6x8 GLCD
	//font and a background equal to the text colour is left out here as well
	GFX.setTextColor(color, bg);
	GFX.setTextSize(size);
#endif
#if SCREENBUFFER == 0
	//Straight to the display every character would be a write transaction of its own, and
	//the chip select sits on the I/O expander: that is I2C traffic per character. One
	//transaction for the whole text instead. Into a buffer nothing is sent, so nothing to do
	SCREEN.startWrite();
#endif
    const char* start = str;
	while (*str && (str - start < len))
	{
		if (*str == '\n')
		{
			cursorY += 9 * size;
			cursorX = x;
			str++;
			continue;
		}
#if LOVYANGFX
		GFX.drawChar((uint8_t)*str, cursorX, cursorY);
#else
		GFX.drawChar(cursorX, cursorY, *str, color, bg, size);
#endif
		cursorX += 6 * size;
		str++;
	}
#if SCREENBUFFER == 0
	SCREEN.endWrite();
#endif
}
