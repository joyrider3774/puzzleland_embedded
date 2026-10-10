#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "bandrender.h"
//a card build takes a strip of the background off the card, see cardimages.h
#include "cardimages.h"
//the strips hold pictures of the black & white skin too, and those are one bit a pixel
#include "onebitimage.h"

//Only where drawing really reaches the display as it happens. A device that draws off screen is
//not flickering to begin with, and there the strips would only cost two more passes over every
//pixel, see PLATFORM_OFFSCREEN_DRAW in Platform.h
#if (SCREENBUFFER == 0) && !PLATFORM_OFFSCREEN_DRAW

//A strip is as tall as a block, so a row of the board is one strip and every block lands in a
//single one of them. Half a block's height would walk every block twice
#define BANDHEIGHT BlockHeight

//the strip being put together, at most the whole width of the screen
static uint16_t* bandBuf = NULL;
//For every row of the screen: where in the encoded background the row starts, and how many
//pixels of that control belong to the rows above it. Without this the background could only be
//read from its start, and a strip at the bottom of the screen would decode the whole image
//Only for a build that can still be asked for an RGB565 skin, see ONEBITONLY: a one bit only
//build never reads these and they are the width of the screen twice over
#if !ONEBITONLY && !CARDIMAGES
static uint16_t* bgRowOffset = NULL;
static uint8_t* bgRowUsed = NULL;
//the background bgRowOffset and bgRowUsed were made for
static const uint8_t* bgIndexed = NULL;
#endif

//an encoded background is at most three bytes per pixel (a run of one pixel every time), so
//the offsets of a 128x128 screen stay inside a uint16_t
static_assert(WINDOW_WIDTH * WINDOW_HEIGHT * 3 <= 65535, "the background offsets do not fit in a uint16_t");
//bgRowUsed holds pixels of one control, and a control covers at most 128 of them
static_assert(WINDOW_WIDTH <= 255, "the pixels of a control do not fit in a uint8_t");

//the rectangle being painted and how far down it the strips have come
static const uint8_t* rectBackground = NULL;
static int16_t rectX, rectY, rectW, rectH, rectNextTop;
//where the strip that is open sits on the screen, and whether one is open at all
static int16_t stripX, stripY, stripW, stripH;
static bool stripOpen = false;

//What every strip of the screen held the last time it was sent. A strip that comes out exactly
//as it went out before does not have to go over the wire again, and on a screen like the one
//between the stages most of the picture stands still: the islands and the buildings never move
//and the clouds drift a quarter of a pixel a frame, so their whole part only really changes
//every second or third frame. Only for a whole screen painted from one background, where the
//strips always sit in the same places
#define STRIPCOUNT ((WINDOW_HEIGHT + BANDHEIGHT - 1) / BANDHEIGHT)
static uint32_t stripSum[STRIPCOUNT];
static bool stripSumKnown[STRIPCOUNT];
static const uint8_t* sumBackground = NULL;
static bool sumsUsable = false;

//pixels are little endian RGB565, read a byte at a time because the images are uint8_t arrays
//in flash, which on some devices only takes whole word reads
static inline uint16_t ReadPixel(const uint8_t* p)
{
	return (uint16_t)(PLATFORM_READ_BYTE(p) | (PLATFORM_READ_BYTE(p + 1) << 8));
}

//walks the background once and writes down where every row of the screen starts
#if !ONEBITONLY && !CARDIMAGES
static void IndexBackground(const uint8_t* image)
{
	const uint8_t* data = image;
	uint32_t pixel = 0; //the first pixel this control covers
	int16_t row = 0;
	while (row < WINDOW_HEIGHT)
	{
		const uint8_t control = PLATFORM_READ_BYTE(data);
		const uint32_t count = (uint32_t)(control & 0x7F) + 1;
		//every row that starts inside this control
		while ((row < WINDOW_HEIGHT) && ((uint32_t)row * WINDOW_WIDTH < pixel + count))
		{
			bgRowOffset[row] = (uint16_t)(data - image);
			bgRowUsed[row] = (uint8_t)((uint32_t)row * WINDOW_WIDTH - pixel);
			row++;
		}
		pixel += count;
		data += (control & 0x80) ? 3 : 1 + count * 2;
	}
	bgIndexed = image;
}
#endif

#if ONEBITIMAGES
//The piece of a background packed one bit a pixel that the strip covers. The rows above it are
//passed over by the reader, which holds what is left of a run between them, and every row it
//gives is a bit a pixel: set is white and clear is black, the two colours the skin has
//This stands in for the index the run length encoded background builds, see IndexBackground: a
//plane packed one bit a pixel cannot be entered in the middle, since a row of it may be told to
//repeat the row above. What is kept instead is the reader itself, carried from one strip to the
//next, and the row it stands at. The picture does not scroll, so that row is simply a screen row
//and the strips of a frame are composed from the top down: only the rows between the strip that
//was done last and this one have to be passed over. Starting from the top for every strip meant
//decoding the whole picture once per strip, eight and a half times over for a full screen repaint
static OneBitReader bgPlane;
static const uint8_t* bgPlaneFor = NULL;             //the picture it was started on
static int16_t bgPlaneAt = -1;                       //the screen row it stands at, -1 when unset
//The row it read last, which is also the row it decodes the next one into. A plane packed as rows
//may say the next row is this one again, so the two cannot be separate buffers
static uint8_t bgPlaneRow[ONEBIT_MAX_STRIDE];

static PLATFORM_HOT_CODE void BandBackgroundOneBit(const uint8_t* image)
{
	const int stride = (OneBitWidth(image) + 7) / 8;
	//A strip above the one that was done last cannot be reached by going on, so the picture is
	//taken from the top again. That is every first strip of a frame, and the picture changing
	if ((bgPlaneAt < 0) || (bgPlaneAt > stripY) || (bgPlaneFor != image))
	{
		OneBitReaderInit(&bgPlane, image + ONEBIT_HEADER, OneBitFlags(image), false);
		bgPlaneFor = image;
		bgPlaneAt = 0;
	}
	OneBitReaderSkip(&bgPlane, stripY - bgPlaneAt, stride, bgPlaneRow);
	bgPlaneAt = (int16_t)(stripY + stripH);
	for (int16_t r = 0; r < stripH; r++)
	{
		OneBitReaderRow(&bgPlane, bgPlaneRow, stride);
		uint16_t* dst = &bandBuf[r * stripW];
		for (int16_t c = 0; c < stripW; c++)
			dst[c] = OneBitAt(bgPlaneRow, stripX + c) ? ONEBIT_SET : ONEBIT_CLEAR;
	}
}
#endif

//the piece of the background the strip covers, which is what the strip starts as
static void BandBackground(const uint8_t* image)
{
#if CARDIMAGES
	//The strip is WINDOW_WIDTH by BANDHEIGHT of RGB565, which is what this buffer is, so the
	//background's rows are read from the card straight into it: nothing to decode, no second copy,
	//and no index to carry. This is where nearly all of a card build's bytes are read
	//the whole strip in one read where the rows lie together, see CardImages_Rows
	if (!CardImages_Rows(image, stripX, stripY, stripW, stripH, bandBuf))
		memset(bandBuf, 0, (size_t)stripW * stripH * 2);
#elif ONEBITIMAGES
	//the skin built in keeps its pictures one bit a pixel, so there is no RGB565 to read
	BandBackgroundOneBit(image);
#else
	if (bgIndexed != image)
		IndexBackground(image);

	for (int16_t r = 0; r < stripH; r++)
	{
		const uint8_t* data = image + bgRowOffset[stripY + r];
		uint8_t used = bgRowUsed[stripY + r]; //pixels of this control that lie before the row
		int16_t skip = stripX;                //pixels of the row left of the strip
		int16_t left = stripW;
		uint16_t* dst = &bandBuf[r * stripW];
		while (left > 0)
		{
			const uint8_t control = PLATFORM_READ_BYTE(data);
			const uint8_t count = (uint8_t)((control & 0x7F) + 1);
			const bool run = (control & 0x80) != 0;
			int16_t avail = (int16_t)(count - used);
			if (skip >= avail)
				skip -= avail;
			else
			{
				used = (uint8_t)(used + skip);
				avail -= skip;
				skip = 0;
				if (avail > left)
					avail = left;
				if (run)
				{
					const uint16_t color = ReadPixel(data + 1);
					for (int16_t i = 0; i < avail; i++)
						*dst++ = color;
				}
				else
				{
					//the pixels are little endian RGB565, the same as the strip holds, so the
					//run is copied as it lies instead of a byte at a time
					PLATFORM_READ_BYTES((uint8_t*)dst, data + 1 + used * 2, (size_t)avail * 2);
					dst += avail;
				}
				left -= avail;
			}
			used = 0;
			data += run ? 3 : 1 + count * 2;
		}
	}
#endif
}

//sends the strip that is open to the display, unless it holds exactly what it held last time.
//One transaction and one window per strip: a window that outlives its transaction is not
//something every display class keeps
static void SendStrip(void)
{
	if (sumsUsable)
	{
		//FNV-1a over the strip. Two strips of different pixels landing on the same value would
		//leave one of them a frame behind, which at 32 bits happens about once in four thousand
		//million strips
		uint32_t sum = 2166136261UL;
		const uint16_t count = (uint16_t)(stripW * stripH);
		for (uint16_t i = 0; i < count; i++)
			sum = (sum ^ bandBuf[i]) * 16777619UL;
		const uint16_t index = (uint16_t)(stripY / BANDHEIGHT);
		if (stripSumKnown[index] && (stripSum[index] == sum))
			return;
		stripSum[index] = sum;
		stripSumKnown[index] = true;
	}

	SCREEN.startWrite();
	SCREEN.setAddrWindow(stripX, stripY, stripW, stripH);
#if LOVYANGFX
	//true: the strip holds plain RGB565, the library puts it in display order
	SCREEN.writePixels(bandBuf, (int32_t)stripW * stripH, true);
#else
	SCREEN.pushPixels(bandBuf, stripW * stripH);
#endif
	SCREEN.endWrite();
}

void BandRender_Init(void)
{
	//Put everything that says what the display holds back first, and do it whether or not the
	//memory is already there: a screen that starts paints from nothing, and what the screen
	//before it left in here would make this one skip work it has to do
#if !ONEBITONLY && !CARDIMAGES
	bgIndexed = NULL;
#endif
	sumBackground = NULL;
	sumsUsable = false;
	stripOpen = false;
	for (uint16_t i = 0; i < STRIPCOUNT; i++)
		stripSumKnown[i] = false;

	if (bandBuf)
		return;
	bandBuf = (uint16_t*)malloc((size_t)WINDOW_WIDTH * BANDHEIGHT * sizeof(uint16_t));
#if !ONEBITONLY && !CARDIMAGES
	bgRowOffset = (uint16_t*)malloc((size_t)WINDOW_HEIGHT * sizeof(uint16_t));
	bgRowUsed = (uint8_t*)malloc((size_t)WINDOW_HEIGHT);
#endif
	if (!bandBuf
#if !ONEBITONLY && !CARDIMAGES
	    || !bgRowOffset || !bgRowUsed
#endif
	   )
	{
		//all or nothing, the screens paint the plain way without them
		BandRender_Deinit();
		Platform_Log("no room for the strip buffer, the screen is painted the plain way\n");
		return;
	}
}

void BandRender_Deinit(void)
{
	free(bandBuf);
	bandBuf = NULL;
#if !ONEBITONLY && !CARDIMAGES
	free(bgRowOffset);
	free(bgRowUsed);
	bgRowOffset = NULL;
	bgRowUsed = NULL;
	bgIndexed = NULL;
#endif
	stripOpen = false;
	sumBackground = NULL;
	sumsUsable = false;
}

bool BandRender_Ready(void)
{
	return bandBuf != NULL;
}

bool BandRender_Begin(const uint8_t* background, int16_t x, int16_t y, int16_t w, int16_t h)
{
	if (!bandBuf || !background)
		return false;
	if (x < 0) { w = (int16_t)(w + x); x = 0; }
	if (y < 0) { h = (int16_t)(h + y); y = 0; }
	if (x + w > WINDOW_WIDTH) w = (int16_t)(WINDOW_WIDTH - x);
	if (y + h > WINDOW_HEIGHT) h = (int16_t)(WINDOW_HEIGHT - y);
	if ((w <= 0) || (h <= 0))
		return false;
	rectBackground = background;
	rectX = x;
	rectY = y;
	rectW = w;
	rectH = h;
	rectNextTop = y;
	stripOpen = false;

	//the strips only keep their places when the whole screen is painted from one background.
	//Anything else (a piece of the board, one character of the clock) starts them again
	sumsUsable = (x == 0) && (y == 0) && (w == WINDOW_WIDTH) && (h == WINDOW_HEIGHT);
	if (!sumsUsable || (background != sumBackground))
	{
		for (uint16_t i = 0; i < STRIPCOUNT; i++)
			stripSumKnown[i] = false;
		sumBackground = sumsUsable ? background : NULL;
	}
	return true;
}

bool BandRender_Next(void)
{
	//what was drawn into the strip before this one goes out now
	if (stripOpen)
	{
		stripOpen = false;
		SendStrip();
	}
	if (rectNextTop >= rectY + rectH)
		return false;
	stripX = rectX;
	stripW = rectW;
	stripY = rectNextTop;
	stripH = (int16_t)((rectNextTop + BANDHEIGHT <= rectY + rectH) ? BANDHEIGHT
	                                                              : (rectY + rectH - rectNextTop));
	rectNextTop = (int16_t)(rectNextTop + BANDHEIGHT);
#if CHGAME_TIMING
	const uint32_t tSection = Platform_Micros();
#endif
	BandBackground(rectBackground);
#if CHGAME_TIMING
	bandBgUs += Platform_Micros() - tSection;
#endif
	stripOpen = true;
	return true;
}

int16_t BandRender_StripX(void) { return stripX; }
int16_t BandRender_StripY(void) { return stripY; }
int16_t BandRender_StripW(void) { return stripW; }
int16_t BandRender_StripH(void) { return stripH; }

bool BandRender_Drawing(void)
{
	return stripOpen;
}

void BandRender_Fill(int x, int y, int w, int h, uint16_t color)
{
	int x0 = (x < stripX) ? stripX : x;
	int y0 = (y < stripY) ? stripY : y;
	int x1 = x + w, y1 = y + h;
	if (x1 > stripX + stripW) x1 = stripX + stripW;
	if (y1 > stripY + stripH) y1 = stripY + stripH;
	for (int row = y0; row < y1; row++)
	{
		uint16_t* dst = &bandBuf[(row - stripY) * stripW + (x0 - stripX)];
		for (int col = x0; col < x1; col++)
			*dst++ = color;
	}
}

//the w by h part at sx,sy of a raw RGB565 image that is dataWidth pixels wide, at x,y
void BandRender_Image(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth)
{
	if (!data)
		return;
	//where the image's top left corner lands on the screen
	const int ox = x - sx, oy = y - sy;
	int c0 = sx, c1 = sx + w, r0 = sy, r1 = sy + h;
	if (c0 < stripX - ox) c0 = stripX - ox;
	if (r0 < stripY - oy) r0 = stripY - oy;
	if (c1 > stripX + stripW - ox) c1 = stripX + stripW - ox;
	if (r1 > stripY + stripH - oy) r1 = stripY + stripH - oy;
	if ((c0 >= c1) || (r0 >= r1))
		return;
	for (int r = r0; r < r1; r++)
	{
		uint16_t* dst = &bandBuf[(oy + r - stripY) * stripW + (ox + c0 - stripX)];
		const uint8_t* src = data + ((r * dataWidth) + c0) * 2;
		PLATFORM_READ_BYTES((uint8_t*)dst, src, (size_t)(c1 - c0) * 2);
	}
}

#if CARDIMAGES
//A picture from the card into the strip, see cardimages.h. The rows are read with nothing of the
//display's open, which is the bus rule a shared bus imposes, and the strip goes out afterwards;
//that is what makes a strip the right place to draw a card build's pictures
void BandRender_ImageCard(int x, int y, int sx, int sy, int w, int h, const uint8_t* data,
                          bool transparent)
{
	if (!data || (w <= 0) || (h <= 0))
		return;
	const int ox = x - sx, oy = y - sy;
	int c0 = sx, c1 = sx + w, r0 = sy, r1 = sy + h;
	if (c0 < stripX - ox) c0 = stripX - ox;
	if (r0 < stripY - oy) r0 = stripY - oy;
	if (c1 > stripX + stripW - ox) c1 = stripX + stripW - ox;
	if (r1 > stripY + stripH - oy) r1 = stripY + stripH - oy;
	if ((c0 >= c1) || (r0 >= r1))
		return;
	const int cols = c1 - c0;
	//A picture small enough to be kept whole sits in RAM, and its rows are copied out of it
	//rather than asked for one at a time: that is the same copy a flash build makes, and for
	//a sheet drawn hundreds of times a frame it is most of what a strip costs
	const uint8_t* px = CardImages_Cached(data);
	const int pitch = px ? (int)CardImages_Width(data) : 0;
	for (int r = r0; r < r1; r++)
	{
		uint16_t* dst = &bandBuf[(oy + r - stripY) * stripW + (ox + c0 - stripX)];
		uint16_t scratch[WINDOW_WIDTH];
		const uint16_t* src;
		if (px)
			src = (const uint16_t*)(px + ((size_t)r * pitch + c0) * sizeof(uint16_t));
		else if (CardImages_Row(data, c0, r, cols, scratch))
			src = scratch;
		else
		{
			//the row did not come, so nothing of it is drawn
			continue;
		}
		if (!transparent)
		{
			//nothing to leave out, so the row lands in the strip where it belongs
			memcpy(dst, src, (size_t)cols * sizeof(uint16_t));
			continue;
		}
		//the transparent pixels keep what the strip already holds
		for (int c = 0; c < cols; c++)
			if (src[c] != COLOR_TRANSPARENT)
				dst[c] = src[c];
	}
}
#endif

#if ONEBITIMAGES
//A one bit picture into the strip, see onebitimage.h. The strip holds plain RGB565, so the bits
//are spread into it as the two colours they stand for, and the mask says which pixels to leave
PLATFORM_HOT_CODE void BandRender_ImageOneBit(int x, int y, int sx, int sy, int w, int h, const uint8_t* data,
                            bool transparent)
{
	if (!data || (w <= 0) || (h <= 0))
		return;
	const int dataWidth = OneBitWidth(data);
	const int dataHeight = OneBitHeight(data);
	const int maskAt = OneBitMaskAt(data);
	const bool useMask = transparent && (maskAt != 0);
	//how each plane is stored, see tools/onebit.py
	const int flags = OneBitFlags(data);

	const int ox = x - sx, oy = y - sy;
	int c0 = sx, c1 = sx + w, r0 = sy, r1 = sy + h;
	if (c0 < 0) c0 = 0;
	if (r0 < 0) r0 = 0;
	if (c1 > dataWidth) c1 = dataWidth;
	if (r1 > dataHeight) r1 = dataHeight;
	//and then to the strip, which is the only part of the screen this writes to
	if (c0 < stripX - ox) c0 = stripX - ox;
	if (r0 < stripY - oy) r0 = stripY - oy;
	if (c1 > stripX + stripW - ox) c1 = stripX + stripW - ox;
	if (r1 > stripY + stripH - oy) r1 = stripY + stripH - oy;
	if ((c0 >= c1) || (r0 >= r1))
		return;

	const int stride = (dataWidth + 7) / 8;
	//the rows come first: skipping fills them, a row being able to say it is the one above
	uint8_t rowPixels[ONEBIT_MAX_STRIDE];
	uint8_t rowMask[ONEBIT_MAX_STRIDE];
	OneBitReader pixels;
	OneBitReaderInit(&pixels, data + ONEBIT_HEADER, flags, false);
	OneBitReaderSkip(&pixels, r0, stride, rowPixels);
	OneBitReader mask;
	if (useMask)
	{
		OneBitReaderInit(&mask, data + maskAt, flags, true);
		OneBitReaderSkip(&mask, r0, stride, rowMask);
	}
	for (int r = r0; r < r1; r++)
	{
		OneBitReaderRow(&pixels, rowPixels, stride);
		if (useMask)
			OneBitReaderRow(&mask, rowMask, stride);
		uint16_t* dst = &bandBuf[(oy + r - stripY) * stripW + (ox + c0 - stripX)];
		for (int c = c0; c < c1; c++, dst++)
		{
			if (useMask && !OneBitAt(rowMask, c))
				continue;
			*dst = OneBitAt(rowPixels, c) ? ONEBIT_SET : ONEBIT_CLEAR;
		}
	}
}
#endif

//The same for a run length encoded image, the format tools/png2rle565.py writes. The stream can
//only be read from its start, so it is walked until the last row that is wanted; the controls
//before that cost a step each, not a pixel each
void BandRender_ImageRLE(int x, int y, int sx, int sy, int w, int h, const uint8_t* data,
                         int dataWidth, int dataHeight, bool transparent)
{
	if (!data || (w <= 0) || (h <= 0))
		return;
	const int ox = x - sx, oy = y - sy;
	//the columns and rows of the image that land in the strip
	int c0 = sx, c1 = sx + w, r0 = sy, r1 = sy + h;
	if (c0 < 0) c0 = 0;
	if (r0 < 0) r0 = 0;
	if (c1 > dataWidth) c1 = dataWidth;
	if (r1 > dataHeight) r1 = dataHeight;
	if (c0 < stripX - ox) c0 = stripX - ox;
	if (r0 < stripY - oy) r0 = stripY - oy;
	if (c1 > stripX + stripW - ox) c1 = stripX + stripW - ox;
	if (r1 > stripY + stripH - oy) r1 = stripY + stripH - oy;
	if ((c0 >= c1) || (r0 >= r1))
		return;

	uint32_t left = (uint32_t)dataWidth * dataHeight;
	int cx = 0, cy = 0;
	while ((left > 0) && (cy < r1))
	{
		const uint8_t control = PLATFORM_READ_BYTE(data++);
		uint16_t count = (uint16_t)((control & 0x7F) + 1);
		if (count > left)
			count = (uint16_t)left;
		const bool run = (control & 0x80) != 0;
		uint16_t color = 0;
		const uint8_t* literal = NULL;
		if (run)
		{
			color = ReadPixel(data);
			data += 2;
		}
		else
		{
			literal = data;
			data += (uint32_t)count * 2;
		}
		left -= count;
		//the pixels of the control a row at a time, only the part inside the strip is written
		for (uint16_t done = 0; done < count; )
		{
			int n = dataWidth - cx;
			if (n > (int)(count - done))
				n = (int)(count - done);
			if ((cy >= r0) && (cy < r1))
			{
				const int a = (cx > c0) ? cx : c0;
				const int e = (cx + n < c1) ? cx + n : c1;
				if (a < e)
				{
					uint16_t* dst = &bandBuf[(oy + cy - stripY) * stripW + (ox + a - stripX)];
					if (!transparent && !run)
					{
						//nothing to leave out, so the pixels are copied as they lie
						PLATFORM_READ_BYTES((uint8_t*)dst, literal + ((int)done + (a - cx)) * 2,
						                    (size_t)(e - a) * 2);
					}
					else
					{
						for (int c = a; c < e; c++, dst++)
						{
							const uint16_t pixel = run ? color
							                           : ReadPixel(literal + ((int)done + (c - cx)) * 2);
							if (!transparent || (pixel != COLOR_TRANSPARENT))
								*dst = pixel;
						}
					}
				}
			}
			done = (uint16_t)(done + n);
			cx += n;
			if (cx >= dataWidth)
			{
				cx = 0;
				cy++;
			}
		}
	}
}

#else

//with a screen buffer the whole frame is put together off screen and sent in one go anyway
void BandRender_Init(void) {}
void BandRender_Deinit(void) {}
bool BandRender_Ready(void) { return false; }
bool BandRender_Begin(const uint8_t* background, int16_t x, int16_t y, int16_t w, int16_t h)
{
	(void)background; (void)x; (void)y; (void)w; (void)h;
	return false;
}
bool BandRender_Next(void) { return false; }
int16_t BandRender_StripX(void) { return 0; }
int16_t BandRender_StripY(void) { return 0; }
int16_t BandRender_StripW(void) { return 0; }
int16_t BandRender_StripH(void) { return 0; }
bool BandRender_Drawing(void) { return false; }
void BandRender_Fill(int x, int y, int w, int h, uint16_t color)
{
	(void)x; (void)y; (void)w; (void)h; (void)color;
}
void BandRender_Image(int x, int y, int sx, int sy, int w, int h, const uint8_t* data, int dataWidth)
{
	(void)x; (void)y; (void)sx; (void)sy; (void)w; (void)h; (void)data; (void)dataWidth;
}
#if ONEBITIMAGES
void BandRender_ImageOneBit(int x, int y, int sx, int sy, int w, int h, const uint8_t* data,
                            bool transparent)
{
	(void)x; (void)y; (void)sx; (void)sy; (void)w; (void)h; (void)data; (void)transparent;
}
#endif
void BandRender_ImageRLE(int x, int y, int sx, int sy, int w, int h, const uint8_t* data,
                         int dataWidth, int dataHeight, bool transparent)
{
	(void)x; (void)y; (void)sx; (void)sy; (void)w; (void)h; (void)data;
	(void)dataWidth; (void)dataHeight; (void)transparent;
}

#endif
