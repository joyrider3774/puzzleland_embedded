#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "sound.h"
#include "cpasswordselector.h"


CPasswordSelector* CPasswordSelector_Create()
{
	CPasswordSelector* Result = (CPasswordSelector*) malloc(sizeof(CPasswordSelector));
	Result->X = 0;
	Result->Y = 0;
	return Result;
}

void CPasswordSelector_Draw(CPasswordSelector* Selector)
{
	//the arrow is the rightmost six pixels of the image and points right, so the image is put
	//a whole width in front of the letter and the arrow ends up just left of it
	drawImageRLETransparent(XOffsetPassword + Selector->X * PasswordCellWidth - selectWidth - 2,
	                        YOffsetPassword + Selector->Y * PasswordCellHeight - 3, selectWidth, selectHeight, ImgSelect);
}

void CPasswordSelector_MoveDown(CPasswordSelector* Selector)
{
	if (Selector->Y < LetterRows - 2)
	{
		Selector->Y = Selector->Y + 1;
		playMenuSound();
	}
	else
		if ((Selector->Y == LetterRows -2) && (Selector->X < LetterCols -2))
		{
			Selector->Y = Selector->Y + 1;
			playMenuSound();
		}
}

void CPasswordSelector_MoveLeft(CPasswordSelector* Selector)
{
	if (Selector->X > 0)
	{
		Selector->X = Selector->X -1;
		playMenuSound();
	}
}

void CPasswordSelector_MoveRight(CPasswordSelector* Selector)
{
	if (Selector->Y < LetterRows -1)
		if (Selector->X < LetterCols -1)
		{
			Selector->X = Selector->X + 1;
			playMenuSound();
		}

	if (Selector->Y == LetterRows - 1)
		if (Selector->X < LetterCols -3)
		{
			Selector->X = Selector->X + 1;
			playMenuSound();
		}
}

void CPasswordSelector_MoveUp(CPasswordSelector* Selector)
{
	if (Selector->Y > 0)
	{
		Selector->Y = Selector->Y - 1;
		playMenuSound();
	}
}

uint8_t CPasswordSelector_GetX(CPasswordSelector* Selector)
{
	return Selector->X;
}

uint8_t CPasswordSelector_GetY(CPasswordSelector* Selector)
{
	return Selector->Y;
}

void CPasswordSelector_Destroy(CPasswordSelector* Selector)
{
	free(Selector);
}
