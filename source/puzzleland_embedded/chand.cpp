#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "chand.h"

CHand* CHand_Create()
{
	CHand* Result = (CHand*)malloc(sizeof(CHand));
	Result->Hidden = false;
	Result->X = 0;
	Result->Y = 0;
	Result->MoveCoolDown = 0;
	return Result;
}

void CHand_Draw(CHand* Hand)
{
	if (!Hand->Hidden)
	{
		drawImageRLETransparent(MinPlayAreaX + Hand->X * BlockWidth, MinPlayAreaY + Hand->Y * BlockHeight, handWidth, handHeight, ImgHand);
	}
}

void CHand_Move(CHand* Hand)
{
	if (Hand->MoveCoolDown > 0)
		Hand->MoveCoolDown--;
	if ((!Hand->Hidden) && (Hand->MoveCoolDown == 0))
	{
		if (currButtons & BUTTON_UP)
		{
			if (Hand->Y - 1 >= 0)
			{
				Hand->Y = Hand->Y - 1;
				Hand->MoveCoolDown = 3;
			}
		}
		else if (currButtons & BUTTON_DOWN)
		{
			if (Hand->Y + 1 < Rows)
			{
				Hand->Y = Hand->Y + 1;
				Hand->MoveCoolDown = 3;
			}
		}
		
		if (currButtons & BUTTON_LEFT)
		{
			if (Hand->X - 1 >= 0)
			{
				Hand->X = Hand->X - 1;
				Hand->MoveCoolDown = 3;
			}
		}
		else if (currButtons & BUTTON_RIGHT)
		{
			if (Hand->X + 1 < Cols)
			{
				Hand->X = Hand->X + 1;
				Hand->MoveCoolDown = 3;
			}
		}
	}
}

void CHand_SetPosition(CHand* Hand, const int XIn,const int YIn)
{
	Hand->X = XIn;
	Hand->Y = YIn;
}

int CHand_GetPlayFieldX(CHand* Hand)
{
	return Hand->X;
}

int CHand_GetPlayFieldY(CHand* Hand)
{
	return Hand->Y;
}

void CHand_Hide(CHand* Hand)
{
	Hand->Hidden = true;
}

void CHand_Show(CHand* Hand)
{
	Hand->Hidden = false;
}

void CHand_Destroy(CHand* Hand)
{
	free(Hand);
}
