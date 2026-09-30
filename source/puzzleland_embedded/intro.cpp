#include <stdint.h>
#include "gamecommon.h"
#include "helperfuncs.h"
#include "commonvars.h"
#include "intro.h"

//the whole of this is the one picture, so without it there is nothing here to build
#if INTROSCREEN

//when the picture went up, the screen is left a second after. The moment it started and not the
//moment it ends is kept: adding to the clock can carry past what a uint32_t holds, and the
//comparison would then be true straight away and the picture never seen
uint32_t screenDelay = 0;

void IntroInit()
{
	screenDelay = Platform_Micros();
}

void IntroDeInit()
{
}

void Intro()
{
    if(GameState == GSIntroInit)
	{
		IntroInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}

	//nothing moves here, the picture is painted once and stands until the screen is left
	if (needRedraw)
	{
		drawImageRLE(0,0,fullScreenWidth,fullScreenHeight,ImgIntro);
		needRedraw = 0;
	}

	//the subtraction is right even over the wrap, where the comparison of two moments is not
	if(Platform_Micros() - screenDelay > 1000000)
		GameState = GSTitleScreenInit;

	if(GameState == GSIntro)
	{
		if (((currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A))) ||
			((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B))))
			GameState = GSTitleScreenInit;
	}

	if(GameState != GSIntro)
		IntroDeInit();
}

#endif
