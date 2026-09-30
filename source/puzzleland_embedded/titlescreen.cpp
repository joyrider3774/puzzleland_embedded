#include <inttypes.h>
#include "helperfuncs.h"
#include "sound.h"
#include "ctitlescreenselector.h"
#include "commonvars.h"
#include "gamecommon.h"
#include "titlescreen.h"

CTitleScreenSelector* TitleScreenSelector;

void TitleScreenInit()
{
	TitleScreenSelector = CTitleScreenSelector_Create();
}

void TitleScreenDeInit()
{
	CTitleScreenSelector_destroy(TitleScreenSelector);
}

void TitleScreen()
{
	if(GameState == GSTitleScreenInit)
	{
		TitleScreenInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}

	//the arrows are the only thing that moves here, so the screen is only painted again when
	//the selection really changed: at the ends of the menu a press moves nothing
	const uint8_t wasSelected = TitleScreenSelector->Selection;

	if ((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP))
		CTitleScreenSelector_MoveUp(TitleScreenSelector);

	if ((currButtons & BUTTON_DOWN) && !(prevButtons & BUTTON_DOWN))
		CTitleScreenSelector_MoveDown(TitleScreenSelector);

	if (TitleScreenSelector->Selection != wasSelected)
		needRedraw = 1;

	if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
	{
		playMenuSelectSound();
		switch (TitleScreenSelector->Selection)
		{
			case 1:
				//the room before the first this build has, which the next stage steps on from
				Level = FIRSTLEVEL;
				GameState = GSOldManSpeakingInit;
				break;
			case 2:
				GameState = GSPasswordEntryInit;
				break;
			case 3:
				GameState = GSOptionsInit;
				break;
			case 4:
				GameState = GSCreditsInit;
				break;
		}
	}
	if (needRedraw)
	{
		drawImageRLE(0,0,fullScreenWidth,fullScreenHeight, ImgTitle);
		CTitleScreenSelector_Draw(TitleScreenSelector);
		needRedraw = 0;
	}

	if(GameState != GSTitleScreen)
		TitleScreenDeInit();
}
