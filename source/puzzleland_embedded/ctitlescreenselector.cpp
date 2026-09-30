#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "sound.h"
#include "commonvars.h"
#include "ctitlescreenselector.h"


CTitleScreenSelector* CTitleScreenSelector_Create()
{
	CTitleScreenSelector* Result = (CTitleScreenSelector*) malloc(sizeof(CTitleScreenSelector));
    Result->Selection = 1;
	return Result;
}

void CTitleScreenSelector_MoveUp(CTitleScreenSelector* selector)
{

	if (selector->Selection > 1)
	{
		selector->Selection--;
		playMenuSound();
	}

}

void CTitleScreenSelector_MoveDown(CTitleScreenSelector* selector)
{
	if (selector->Selection < 4)
	{
		selector->Selection++;
		playMenuSound();
	}

}

void CTitleScreenSelector_Draw(CTitleScreenSelector* selector)
{
	//the rows of the menu in the title image, which is 128 pixels high: New Game at 32, Password
	//at 51, Options at 71 and Credits at 91, each of them 10 or 11 pixels of text. The selector is
	//the pair of arrows that stands on either side of the row, so it is centred on the text
	int y = 34;
	switch (selector->Selection)
	{
		case 1 : y = 34;
				 break;
		case 2 : y = 53;
				 break;
		case 3 : y = 72;
				 break;
		case 4 : y = 93;
				 break;
	}
	drawImageRLETransparent((WINDOW_WIDTH - titleSelectorWidth) >> 1, y, titleSelectorWidth, titleSelectorHeight, ImgTitleSelector);
}

void CTitleScreenSelector_destroy(CTitleScreenSelector* selector)
{
	free(selector);
}

