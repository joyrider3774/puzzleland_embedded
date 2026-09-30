#include <stdint.h>
#include <stdlib.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "cplayer.h"

//Player anims
const int AnimLeft[6]= {8,9,10,11,12,13};
const int AnimRight[6]  = {1,2,3,4,5,6};
const int AnimEnterBuilding[4]  = {15,16,17,18};


CPlayer* CPlayer_Create(const uint8_t Xin,const uint8_t Yin,const uint8_t MinXin, const uint8_t MaxXin)
{
	CPlayer* Result = (CPlayer*)malloc(sizeof(CPlayer));
 	Result->Y = Yin;
	Result->X = Xin;
 	Result->AnimCounter = 0;
 	Result->Delay = 0;
 	Result->AnimPhase = 1;
 	Result->MaxX = MaxXin;
 	Result->MinX = MinXin;
 	Result->State = Waiting;
	return Result;
}

void CPlayer_Draw(CPlayer* Player)
{
	drawImageRLETransparent(Player->X + 1, Player->Y + ryfPlayerHeight, ryfShadowWidth, ryfShadowHeight, ImgShadow);
	drawImageRLEPart(Player->X, Player->Y, 0, Player->AnimPhase * ryfPlayerHeight, ryfPlayerWidth, ryfPlayerHeight, ImgPlayer, ryfPlayerWidth, ryfPlayerHeight*19, true);
}

uint8_t CPlayer_GetX(CPlayer* Player)
{
	return Player->X;
}

uint8_t CPlayer_GetY(CPlayer* Player)
{
	return Player->Y;
}

uint8_t CPlayer_GetWidth(CPlayer* Player)
{
	return ryfPlayerWidth;
}

void CPlayer_Destroy(CPlayer* Player)
{
 	free(Player);
}

void CPlayer_Move(CPlayer* Player)
{

	if ((Player->State == Walking) || (Player->State==Waiting))
	{
		if (currButtons & BUTTON_LEFT)
		{
			if(Player->X - 2 >= Player->MinX)
			{
				Player->X = Player->X -2;
				Player->AnimPhase = AnimLeft[Player->AnimCounter];
				Player->State = Walking;
			}
			else
				Player->State = Waiting;
		}
 		else
			if (currButtons & BUTTON_RIGHT)
			{
				if(Player->X + 2 <= Player->MaxX)
				{
					Player->X = Player->X +2;
					Player->AnimPhase = AnimRight[Player->AnimCounter];
					Player->State = Walking;
				}
				else
					Player->State = Waiting;
			}
			else
				Player->State = Waiting;
	}
	if (Player->State == Walking)
	{
		if (Player->Delay > 0)
			Player->Delay--;
		if (Player->Delay <= 0)
		{
			Player->AnimCounter++;
			Player->Delay = 3;
		}
     	if (Player->AnimCounter > 5)
     		Player->AnimCounter = 0;
    }
  	if (Player->State == EnterBuilding)
  	{
  		Player->Delay--;
  		if(Player->Delay<=0)
  		{
  			Player->AnimCounter++;
  			Player->Y = Player->Y - 1;
  			Player->Delay = 3;
  		}
  		if (Player->AnimCounter > 3)
  		{
  			Player->AnimCounter = 0;
			if(Player->Y <=87)
				Player->State = EnteredBuilding;
		}
 		Player->AnimPhase = AnimEnterBuilding[Player->AnimCounter];
	}
	if (Player->State == Waiting)
	{
		if ((Player->AnimPhase != 1) && (Player->AnimPhase !=8))
		{
			if (Player->AnimPhase==14)
			{
				Player->AnimPhase = 1;
			}
			else
			{
				if (Player->AnimPhase < 7)
					Player->AnimPhase = 1;
				else
					Player->AnimPhase=8;
			}
		}
	}
 	if (Player->State ==LookingUp)
 		Player->AnimPhase = 14;
}
