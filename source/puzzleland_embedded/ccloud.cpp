#include <stdlib.h>
#include "commonvars.h"
#include "helperfuncs.h"
#include "ccloud.h"

CCloud* CCloud_Create(const uint8_t XIn,const uint8_t YIn,float XiIn,CloudStyles Style)
{
	CCloud* Result = (CCloud*)malloc(sizeof(CCloud));
	if (Style == Big)
	{
		Result->Image = ImgRyfCloud;
		Result->Width = ryfCloudWidth;
		Result->Height = ryfCloudHeight;
	}
	else
	{
		Result->Image = ImgRyfSmallCloud;
		Result->Width=ryfSmallCloudWidth;
		Result->Height=ryfSmallCloudHeight;
	 }
	Result->X = (float)XIn;
	Result->Y = YIn;
	Result->Xi = XiIn;
	return Result;
}

void CCloud_Draw(CCloud* Cloud)
{
	 drawImageRLETransparent((int)Cloud->X, Cloud->Y, Cloud->Width, Cloud->Height, Cloud->Image);
}

void CCloud_Move(CCloud* Cloud)
{
	if ((Cloud->X > -Cloud->Width) && (Cloud->X < WINDOW_WIDTH))
		Cloud->X = Cloud->X + Cloud->Xi;
	else
		if (Cloud->X <= -Cloud->Width)
			Cloud->X = 127;
		else
			Cloud->X = -Cloud->Width + 1.0f;
}


void CCloud_Destroy(CCloud* Cloud)
{
	free(Cloud);
}
