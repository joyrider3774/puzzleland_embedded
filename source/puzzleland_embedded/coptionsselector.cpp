#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "sound.h"
#include "coptionsselector.h"


COptionsSelector* COptionsSelector_Create()
{
	COptionsSelector* Result = (COptionsSelector*)malloc(sizeof(COptionsSelector));
	Result->Selection = 1;
	return Result;
}

void COptionsSelector_Draw(COptionsSelector* Selector)
{
	//the two rows of the options screen, SOUND at 24 and MUSIC at 34, see options.cpp. The arrow
	//of the image points right and sits in its left half, so it stands in front of the row
	int y = 24;
	switch(Selector->Selection)
	{
		case 1 :
			y = 24;
			break;
		case 2 :
			y = 34;
			break;
	}
	//the widest row is "SOUND: OFF", ten characters of six pixels, centred on the screen. The
	//arrow is the leftmost six pixels of the image, so it lands just in front of the row
	drawImageRLETransparent(((WINDOW_WIDTH - 10 * 6) >> 1) - 8, y - 2, optionSelectWidth, optionSelectHeight, ImgOptionSelect);
}

uint8_t COptionsSelector_GetSelection(COptionsSelector* Selector)
{
	return Selector->Selection;
}

void COptionsSelector_MoveDown(COptionsSelector* Selector)
{
	if (Selector->Selection < 1)
	{
		Selector->Selection++;
		playMenuSound();
	}
}

void COptionsSelector_MoveUp(COptionsSelector* Selector)
{
	if (Selector->Selection > 1)
	{
		Selector->Selection--;
		playMenuSound();
	}
}

void COptionsSelector_Destroy(COptionsSelector* Selector)
{
	free(Selector);
}
