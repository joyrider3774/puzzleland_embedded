#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "cstageblock.h"


CStageBlock* CStageBlock_Create()
{
	CStageBlock* Result = (CStageBlock*) malloc(sizeof(CStageBlock));
	Result->Image=NULL;
 	return Result;
}

void CStageBlock_Load(CStageBlock* stageBlock, const uint8_t BlockNr)
{
	switch(BlockNr)
	{
		case 1:
			stageBlock->Image = ImgStageBlock1;
			stageBlock->Width = stageBlock1Width;
			stageBlock->Height = stageBlock1Height;
			break;
		case 2:
			stageBlock->Image = ImgStageBlock2;
			stageBlock->Width = stageBlock2Width;
			stageBlock->Height = stageBlock2Height;
			break;
		case 3:
			stageBlock->Image = ImgStageBlock3;
			stageBlock->Width = stageBlock3Width;
			stageBlock->Height = stageBlock3Height;
			break;
		case 4:
			stageBlock->Image = ImgStageBlock4;
			stageBlock->Width = stageBlock4Width;
			stageBlock->Height = stageBlock4Height;
			break;
		case 5:
			stageBlock->Image = ImgStageBlock5;
			stageBlock->Width = stageBlock5Width;
			stageBlock->Height = stageBlock5Height;
			break;
		case 6:
			stageBlock->Image = ImgStageBlock6;
			stageBlock->Width = stageBlock6Width;
			stageBlock->Height = stageBlock6Height;
			break;
		case 7:
			stageBlock->Image = ImgStageBlock7;
			stageBlock->Width = stageBlock7Width;
			stageBlock->Height = stageBlock7Height;
			break;
		case 8:
			stageBlock->Image = ImgStageBlock8;
			stageBlock->Width = stageBlock8Width;
			stageBlock->Height = stageBlock8Height;
			break;
		case 9:
			stageBlock->Image = ImgStageBlock9;
			stageBlock->Width = stageBlock9Width;
			stageBlock->Height = stageBlock9Height;
			break;
		case 10:
			stageBlock->Image = ImgStageBlock10;
			stageBlock->Width = stageBlock10Width;
			stageBlock->Height = stageBlock10Height;
			break;
		default:
			stageBlock->Image = NULL;
			stageBlock->Width = 0;
			stageBlock->Height = 0;
			break;
	}

	stageBlock->Hidden = false;
	stageBlock->Yi = 3;
	stageBlock->X = 64 - (stageBlock->Width / 2);
	stageBlock->Y = -stageBlock->Height;
}

void CStageBlock_Draw(CStageBlock* stageBlock)
{
	if (!stageBlock->Hidden)
	{
		drawImageRLETransparent(stageBlock->X, stageBlock->Y, stageBlock->Width, stageBlock->Height,stageBlock->Image);
	}
}

void CStageBlock_Move(CStageBlock* stageBlock)
{
	if (!stageBlock->Hidden)
		stageBlock->Y = stageBlock->Y + stageBlock->Yi;
}

int16_t CStageBlock_GetY(CStageBlock* stageBlock)
{
	return stageBlock->Y;
}

uint8_t CStageBlock_GetHeight(CStageBlock* stageBlock)
{
	return stageBlock->Height;
}

void CStageBlock_destroy(CStageBlock* stageBlock)
{
	if(!stageBlock)
		return;
	free(stageBlock);
}
