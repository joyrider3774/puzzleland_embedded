#include <string.h>
#include <stdint.h>
#include "helperfuncs.h"
#include "coptionsselector.h"
#include "commonvars.h"
#include "sound.h"
#include "gamecommon.h"
#include "options.h"

COptionsSelector* Selector;

void OptionsInit()
{
	Selector = COptionsSelector_Create();
}

void OptionsDeInit()
{
	COptionsSelector_Destroy(Selector);
	SaveSettings();
}

void Options()
{
	if(GameState == GSOptionsInit)
	{
		OptionsInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}

	if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
	{
		GameState = GSTitleScreenInit;
	}

	if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
	{
		GameState = GSTitleScreenInit;
	}

	if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
	{
		if (COptionsSelector_GetSelection(Selector) == 1)
		{
			setSoundOn(!isSoundOn());
			setMusicOn(isSoundOn());
			SaveSettings();
			needRedraw = 1;
		}

		playMenuSelectSound();
	}

	//only a selection that really moved changes the screen, see needRedraw in commonvars.h
	const uint8_t wasSelected = COptionsSelector_GetSelection(Selector);

	if ((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP))
		COptionsSelector_MoveUp(Selector);

	if ((currButtons & BUTTON_DOWN) && !(prevButtons & BUTTON_DOWN))
		COptionsSelector_MoveDown(Selector);

	if (COptionsSelector_GetSelection(Selector) != wasSelected)
		needRedraw = 1;

	if (needRedraw)
	{
			#if PAPERBACKGROUND
		drawImageRLE(0, 0, fullScreenWidth, fullScreenHeight, ImgPaper);
		#else
		fillRect(0, 0, fullScreenWidth, fullScreenHeight, ColorPaper);
		#endif

		tftPrint((WINDOW_WIDTH >> 1) - ((strlen("OPTIONS")*6) >> 1), 10, "OPTIONS", strlen("OPTIONS"), ColorText,ColorText,1);

		if (isSoundOn())
			tftPrint((WINDOW_WIDTH >> 1) - ((strlen("SOUND: ON")*6) >> 1),24, "SOUND: ON", strlen("SOUND: ON"), ColorText,ColorText,1);
		else
			tftPrint((WINDOW_WIDTH >> 1) - ((strlen("SOUND: OFF")*6) >> 1),24, "SOUND: OFF", strlen("SOUND: OFF"), ColorText,ColorText,1);

		COptionsSelector_Draw(Selector);
		needRedraw = 0;
	}

	if(GameState != GSOptions)
		OptionsDeInit();
}
