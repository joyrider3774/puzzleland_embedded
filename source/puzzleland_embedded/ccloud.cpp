#include <stdlib.h>
#include "commonvars.h"
#include "helperfuncs.h"
#include "ccloud.h"

CCloud* CCloud_Create(const uint8_t XIn,const uint8_t YIn,int16_t XiIn,CloudStyles Style)
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
	Result->X = (int16_t)(XIn * 256);
	Result->Y = YIn;
	Result->Xi = XiIn;
	return Result;
}

void CCloud_Draw(CCloud* Cloud)
{
	 drawImageRLETransparent(CCloud_ScreenX(Cloud), Cloud->Y, Cloud->Width, Cloud->Height, Cloud->Image);
}

void CCloud_Move(CCloud* Cloud)
{
	if ((Cloud->X > -Cloud->Width * 256) && (Cloud->X < WINDOW_WIDTH * 256))
		Cloud->X = Cloud->X + Cloud->Xi;
	else
		if (Cloud->X <= -Cloud->Width * 256)
			Cloud->X = 127 * 256;
		else
			Cloud->X = (int16_t)((-Cloud->Width + 1) * 256);
}


void CCloud_Destroy(CCloud* Cloud)
{
	free(Cloud);
}
