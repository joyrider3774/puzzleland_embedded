#ifndef CSPACESHIP_H
#define CSPACESHIP_H

#include <stdint.h>

typedef struct CSpaceShip CSpaceShip;
struct CSpaceShip
{
 	int16_t X;
};

void CSpaceShip_destroy(CSpaceShip* spaceShip);
int16_t CSpaceShip_GetX(CSpaceShip* spaceShip);
void CSpaceShip_Move(CSpaceShip* spaceShip);
void CSpaceShip_Draw(CSpaceShip* spaceShip);
CSpaceShip* CSpaceShip_Create();

#endif