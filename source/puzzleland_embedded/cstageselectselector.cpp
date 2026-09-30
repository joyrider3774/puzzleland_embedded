#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "sound.h"
#include "commonvars.h"
#include "cstageselectselector.h"


CStageSelectSelector* CStageSelectSelector_Create()
{
	CStageSelectSelector* Result = (CStageSelectSelector*)malloc(sizeof(CStageSelectSelector));
    Result->X = 0;
	Result->Y = 0;
	return Result;
}

void CStageSelectSelector_Draw(CStageSelectSelector* selector)
{
	//the arrow is the rightmost six pixels of the image and points right, so the image starts a
	//whole width in front of the number and the arrow ends up just left of it
	drawImageRLETransparent(XOffsetStageSelect + selector->X * StageSelectCellWidth + 5 - selectWidth,
	                        YOffsetStageSelect + selector->Y * StageSelectCellHeight - 3, selectWidth, selectHeight,ImgSelect);
}

void CStageSelectSelector_MoveDown(CStageSelectSelector* selector)
{
	if (selector->Y < StageSelectRows - 1)
	{
		selector->Y++;
		playMenuSound();
	}
}

void CStageSelectSelector_MoveLeft(CStageSelectSelector* selector)
{
	if (selector->X > 0)
	{
		selector->X--;
		playMenuSound();
	}
}

void CStageSelectSelector_MoveRight(CStageSelectSelector* selector)
{
	if (selector->X < StageSelectCols - 1)
	{
		selector->X++;
		playMenuSound();
	}
}

void CStageSelectSelector_MoveUp(CStageSelectSelector* selector)
{
	if ( selector->Y > 0)
	{
		selector->Y--;
		playMenuSound();
	}
}

uint8_t CStageSelectSelector_GetSelection(CStageSelectSelector* selector)
{
	return  selector->X + selector->Y * StageSelectCols;
}

void CStageSelectSelector_destroy(CStageSelectSelector* selector)
{
	free(selector);
}
