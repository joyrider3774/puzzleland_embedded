#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "cfairy.h"


CFairy* CFairy_Create(const int XIn, const int YIn,const int AnimDelayIn)
{
	CFairy* Result = (CFairy*)malloc(sizeof(CFairy));
	Result->X = XIn;
	Result->Y = YIn;
	Result->AnimPhase = 0;
	Result->Delay = 0;
	Result->Hidden = false;
	Result->AnimDelay = AnimDelayIn;
	return Result;
}

//counts the frames of the wing beat on, true when it turned to the other one: a screen that
//only paints when something moved (see needRedraw in commonvars.h) asks this, the same way
//CCloud and CPlayer are moved and drawn in two steps
bool CFairy_Move(CFairy* Fairy)
{
	if (Fairy->Hidden)
		return false;
	Fairy->Delay++;
	if (Fairy->Delay < Fairy->AnimDelay)
		return false;
	Fairy->AnimPhase++;
	Fairy->Delay = 0;
	if (Fairy->AnimPhase == 2)
		Fairy->AnimPhase = 0;
	return true;
}

void CFairy_Draw(CFairy* Fairy)
{
	if (!Fairy->Hidden)
	{
		//the image is the two frames of the fairy above each other, so the whole of it is
		//13 x 32: with only one frame's height the second frame is cut away and nothing is drawn
		drawImageRLEPart(Fairy->X, Fairy->Y, 0, Fairy->AnimPhase * ryfFairyHeight, ryfFairyWidth, ryfFairyHeight, ImgRyfFairy, ryfFairyWidth, ryfFairyHeight * 2, true);
	}
}

int CFairy_GetX(CFairy* Fairy)
{
	return Fairy->X;
}

void CFairy_Destroy(CFairy* Fairy)
{
	free(Fairy);
}
