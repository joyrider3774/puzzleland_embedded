#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "cspaceship.h"


CSpaceShip* CSpaceShip_Create()
{
	CSpaceShip* Result = (CSpaceShip*) malloc(sizeof(CSpaceShip));
	Result->X=-spaceshipWidth;
	return Result;
}

void CSpaceShip_Draw(CSpaceShip* spaceShip)
{
	drawImageRLETransparent(spaceShip->X, 11, spaceshipWidth, spaceshipHeight, ImgSpaceship);
}

void CSpaceShip_Move(CSpaceShip* spaceShip)
{
	if (spaceShip->X < 352)
		spaceShip->X = spaceShip->X + 3;
}

int16_t CSpaceShip_GetX(CSpaceShip* spaceShip)
{
	return spaceShip->X;
}

void CSpaceShip_destroy(CSpaceShip* spaceShip)
{
	free(spaceShip);
}
