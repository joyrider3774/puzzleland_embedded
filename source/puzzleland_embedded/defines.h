#ifndef defines_h
#define defines_h

//the device comes first: the display library and SCREENBUFFER are device settings, see
//PlatformESPboy.h / PlatformSDL.h
#include "PlatformDevice.h"

//1 = the art is read from a card while the game runs and none of it is in flash, see
//cardimages.h. It needs a device that can read one (PLATFORM_HAS_CARD in Platform.h) and the card
//file tools/mkcard.py writes. Every skin is then on the card in full RGB565 and the game can be
//asked for any of them, which is what flash could never hold: the whole reason only one reduced
//skin is built in is the 50944 bytes a device has for everything
#ifndef CARDIMAGES
#define CARDIMAGES 0
#endif

//How much RAM a card build keeps its art in. A picture small enough to be worth it is read once
//and kept here, so drawing it again is a copy; a full screen one is read a row or a strip at a
//time and never kept. A screen whose pictures do not all fit still draws correctly, it just reads
//them again, which CardImages_Reads() counts. See the arena in cardimages.cpp
//How much of a room is held while it is read off the card. The reader asks for a byte at a
//time and walks a room once, so this only decides how often the card is asked: a room is a few
//hundred bytes encoded, so 64 is a handful of reads and nothing is kept afterwards
#ifndef CARD_LEVEL_CHUNK
#define CARD_LEVEL_CHUNK 64
#endif

#ifndef CARDARENA
#define CARDARENA 3072
#endif


// window size, the ESPboy display
#define WINDOW_WIDTH 128
#define WINDOW_HEIGHT 128

#define GSTitleScreen 1 
#define GSOldManSpeaking 2 
#define GSPasswordEntry 3 
#define GSOptions 4 
#define GSCredits 5 
#define GSIntro 6 
#define GSQuit 7
#define GSGame 8 
#define GSStageClear 9 
#define GSNextStage 10 
#define GSStageSelect 11

#define GSInitDiff 50

#define GSTitleScreenInit (GSTitleScreen + GSInitDiff)
#define GSOldManSpeakingInit (GSOldManSpeaking + GSInitDiff) 
#define GSPasswordEntryInit (GSPasswordEntry + GSInitDiff)
#define GSOptionsInit (GSOptions + GSInitDiff)
#define GSCreditsInit (GSCredits + GSInitDiff)
#define GSIntroInit (GSIntro + GSInitDiff)
#define GSGameInit (GSGame + GSInitDiff)
#define GSStageClearInit (GSStageClear + GSInitDiff)
#define GSNextStageInit (GSNextStage + GSInitDiff)
#define GSStageSelectInit (GSStageSelect + GSInitDiff)

#define BlockCount 13
#define BorderCount 7
#define BlockWidth 6
#define BlockHeight 6
//the board is Cols x Rows of BlockWidth x BlockHeight, 120 x 96, centred sideways on the
//128 pixel screen with the level and the time above it
#define XOffsetGame ((WINDOW_WIDTH - BlockWidth * Cols) / 2)
#define YOffsetGame 21
//the letters of the password screen, LetterCols x LetterRows in cells wide enough for the
//selector arrow to stand in front of a letter
#define PasswordCellWidth 14
#define PasswordCellHeight 12
#define XOffsetPassword ((WINDOW_WIDTH - (PasswordCellWidth * (LetterCols - 1) + 6)) / 2)
#define YOffsetPassword 40
//the 36 rooms, StageSelectCols x StageSelectRows of a two figure number with the selector
//arrow in front of it
#define StageSelectCols 6
#define StageSelectRows 6

//>>> written by tools/convert_levels.py from assets/levelpacks, do not edit by hand
//FIRSTLEVEL and MAXLEVELS: the run of the 36 rooms a build keeps, for a device with not
//the flash for all of them. Several builds whose runs follow one another hold the lot
//between them, and a room keeps its number whichever build it is in, so a password names
//the same room everywhere. 0 rooms means all of them from the first on
#ifndef FIRSTLEVEL
#define FIRSTLEVEL 0
#endif
#ifndef MAXLEVELS
#define MAXLEVELS 0
#endif
//how many rooms the game has in all, whichever of them this build holds
#define LEVELCOUNT 36
//1 while room n is in the build, counted from 0
#define LEVELBUILT(n) (((n) >= FIRSTLEVEL) && ((MAXLEVELS == 0) || \
                       ((n) < FIRSTLEVEL + MAXLEVELS)))
//how many rooms that leaves
#define LEVELSKEPT ((36 <= FIRSTLEVEL) ? 0 : \
                    (((MAXLEVELS == 0) || (36 - FIRSTLEVEL <= MAXLEVELS)) \
                     ? 36 - FIRSTLEVEL : MAXLEVELS))
#if LEVELSKEPT == 0
#error "the run leaves no rooms at all, see FIRSTLEVEL and MAXLEVELS"
#endif
//<<<
#define StageSelectCellWidth 20
#define StageSelectCellHeight 14
#define XOffsetStageSelect 2
#define YOffsetStageSelect 22
#define Cols 20
#define Rows 16
#define MinPlayAreaX XOffsetGame
#define MinPlayAreaY YOffsetGame
#define MaxPlayAreaX (XOffsetGame + BlockWidth * Cols)
#define MaxPlayAreaY (YOffsetGame + BlockHeight * Rows)
#define LetterRows 4
#define LetterCols 7

//the sizes of the images, the same in every skin (helperfuncs.cpp checks them)
#define fullScreenWidth 128
#define fullScreenHeight 128
#define bridgeWidth 57
#define bridgeHeight 8
#define ryfCloudWidth 30
#define ryfCloudHeight 14 
#define ryfSmallCloudWidth 15
#define ryfSmallCloudHeight 7
#define ryfFairyWidth 13
#define ryfFairyHeight 16
#define ryfPlayerWidth 13
#define ryfPlayerHeight 16
#define ryfShadowWidth 10
#define ryfShadowHeight 2
#define optionSelectWidth 13
#define optionSelectHeight 13
#define handWidth 13
#define handHeight 7
#define selectWidth 13
#define selectHeight 13
#define titleSelectorWidth 93
#define titleSelectorHeight 7
#define spaceshipWidth 27
#define spaceshipHeight 19
#define stageClearKaderWidth 125
#define stageClearKaderHeight 49


#define stageBlock1Width 13
#define stageBlock1Height 19
#define stageBlock2Width 31
#define stageBlock2Height 6
#define stageBlock3Width 19
#define stageBlock3Height 19
#define stageBlock4Width 19 
#define stageBlock4Height 13
#define stageBlock5Width 25
#define stageBlock5Height 13
#define stageBlock6Width 19
#define stageBlock6Height 19
#define stageBlock7Width 19
#define stageBlock7Height 19
#define stageBlock8Width 19
#define stageBlock8Height 19
#define stageBlock9Width 19
#define stageBlock9Height 19
#define stageBlock10Width 13
#define stageBlock10Height 13

//the colour images use for transparent pixels, RGB565 of (255,0,255)
#define COLOR_TRANSPARENT 0xF81F

#define skinDefault 0
#define skinBlackWhite 1

//INTROSCREEN: 1 opens the game on a picture of its own for a second, 0 goes straight to the title
//screen. The picture is 1151 bytes of flash and is shown nowhere else
#ifndef INTROSCREEN
#define INTROSCREEN 1
#endif

//BETWEENSTAGEFIRSTPICTURE: 1 gives the first stage a between stage picture of its own, 0 shows
//the one every other stage shows. The first stage's is 937 bytes of flash
#ifndef BETWEENSTAGEFIRSTPICTURE
#define BETWEENSTAGEFIRSTPICTURE 1
#endif

//PAPERBACKGROUND: 1 writes the menus on a picture of a sheet of paper, 0 on plain ColorBackground.
//The picture is 675 bytes of flash
#ifndef PAPERBACKGROUND
#define PAPERBACKGROUND 1
#endif

//FORCESKIN: -1 = the default skin, or the black & white one with a 1 bpp buffer, n = skin n
//(0 default, 1 black & white). There is no skin option in the game, so only the skin used is
//built in. A 1 bpp buffer has only two colours to show, so the black & white skin is the one it
//takes on its own. A build can still ask it for the colour skin, whose shades then go through the
//brightness rule in SetBufferBit, and with DITHERING come out as a pattern of the two colours
//rather than as the nearer of them. Set by the device header or the build
#if !defined(FORCESKIN) || (FORCESKIN < 0)
  #undef FORCESKIN
  #if SCREENBUFFER == 1
  #define FORCESKIN skinBlackWhite
  #else
  #define FORCESKIN skinDefault
  #endif
#endif

//1 = the skin built in is stored one bit a pixel by tools/onebit.py and drawn by the one
//bit routines in onebitimage.cpp. Only the black & white skin is kept that way: it shows
//two colours, and keeping each of them in sixteen bits costs both flash and the work of
//writing a colour per pixel. Only one skin is ever built in, so the choice is known here
//A card build has no skin built in at all, so neither of the flash formats is there
#define ONEBITIMAGES (!CARDIMAGES && (FORCESKIN == skinBlackWhite))

//1 when the black & white skin is the only one in the build. Every picture is then one bit a pixel
//and the paths that read RGB565 are dead: a build that is only ever going to draw one bit pictures
//need not carry the index the run length encoded background is read through, which is a row table
//the width of the screen
#define ONEBITONLY ONEBITIMAGES

#define FRAMERATE 30
//1 = every frame waits until 1/FRAMERATE of a second has passed, 0 = a frame starts as soon
//as the last one is done, to see how fast the game can go. The music counts frames, so
//without the lock it plays faster as well. A build can set it itself
#ifndef FPSLOCK
#define FPSLOCK 1
#endif
//1 = the debug header (frame rate, free heap and stack) is always shown, Up + Down does not
//hide it. 0 = it starts hidden and Up + Down shows and hides it. A build can set it itself
#ifndef FORCEDEBUG
#define FORCEDEBUG 0
#endif
//1 = the colours of an image are spread over the ones the buffer can hold, so that a shade it
//has no colour for is a pattern of the two it does instead of the nearer of them. 0 = every
//colour becomes the nearest one there is, which shows as bands across anything that shades.
//An 8 bpp buffer is RGB332 and drops 2 bits of red, 3 of green and 3 of blue, and a 1 bpp buffer
//keeps only black and white, so both have something to spread. A 16 bpp buffer holds every colour
//of the image as it is and is left alone. A build can set this itself, see DitherSpread in
//Platform.h
#ifndef DITHERING
#define DITHERING 0
#endif

#endif
