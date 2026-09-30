#include <string.h>
#include <stdint.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "sound.h"
#include "gamecommon.h"
#include "credits.h"

size_t CreditsNrOfChars;

void CreditsInit()
{
	//CreateOtherMenuItems();
	CreditsNrOfChars = 0;
	playTextSound();
}

void CreditsDeInit()
{	
	stopTextSound();
}

void Credits()
{
	//the screen is 128 pixels wide and the font is six wide, so a line is at most 20 characters
	const char *Tekst = "Puzzle Land is made\nby Willems Davy\nGraphics made using\ngimp\nSprites by Yann R.\nFernandez\nPuzzle Land is a\nremake of Daedalian\nopus for gameboy by\nVic Tokai Inc. 1990";

	if(GameState == GSCreditsInit)
	{
		CreditsInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}

	if (((currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A))) ||
		((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B))) ||
	    ((currButtons & BUTTON_R) && (!(prevButtons & BUTTON_R))))
		GameState = GSTitleScreenInit;

	if (CreditsNrOfChars < strlen(Tekst))
	{
#if SCREENBUFFER == 0
		//without a screen buffer every drawing call goes straight to the display, so a
		//character at a time would mean painting the whole screen again for every one of
		//them. The text is put there in one go instead
		CreditsNrOfChars = strlen(Tekst);
#else
		//a character more of the text is on the screen, so it is painted again. Once the whole
		//text is there nothing moves any more and the screen is left as it is
		CreditsNrOfChars++;
#endif
		needRedraw = 1;
	}
	else
		stopTextSound();

	if (needRedraw)
	{
			#if PAPERBACKGROUND
		drawImageRLE(0, 0, fullScreenWidth, fullScreenHeight, ImgPaper);
		#else
		fillRect(0, 0, fullScreenWidth, fullScreenHeight, ColorPaper);
		#endif
		tftPrint((WINDOW_WIDTH >> 1) - ((strlen("CREDITS") * 6) >> 1), 10, "CREDITS", strlen("CREDITS"), ColorForeground, ColorForeground, 1);
		tftPrint(7, 22, Tekst, CreditsNrOfChars, ColorText, ColorText, 1);
		needRedraw = 0;
	}

	if (GameState != GSCredits)
		CreditsDeInit();
}
