#ifndef CPLAYER_H
#define CPLAYER_H

#include <stdint.h>

typedef enum {Walking,Waiting,LookingUp,EnterBuilding,EnteredBuilding} PlayerStates;

typedef struct CPlayer CPlayer;
struct CPlayer
{
 	PlayerStates State;
 	uint8_t X,Y,AnimPhase,AnimCounter,MinX,MaxX;
	int8_t Delay;
};

void CPlayer_Move(CPlayer* Player);
void CPlayer_Destroy(CPlayer* Player);
uint8_t CPlayer_GetWidth(CPlayer* Player);
uint8_t CPlayer_GetY(CPlayer* Player);
uint8_t CPlayer_GetX(CPlayer* Player);
void CPlayer_Draw(CPlayer* Player);
CPlayer* CPlayer_Create(const uint8_t Xin,const uint8_t Yin,const uint8_t MinXin, const uint8_t MaxXin);
#endif