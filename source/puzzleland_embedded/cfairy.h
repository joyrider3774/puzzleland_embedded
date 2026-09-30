#ifndef CFAIRY_H
#define CFAIRY_H

#include <stdint.h>

typedef struct CFairy CFairy;
struct CFairy
{
  	bool Hidden;
 	int X,Y,Delay,AnimPhase,AnimDelay;
} ;

void CFairy_Destroy(CFairy* Fairy);
int CFairy_GetX(CFairy* Fairy);
bool CFairy_Move(CFairy* Fairy);
void CFairy_Draw(CFairy* Fairy);
CFairy* CFairy_Create(const int XIn, const int YIn,const int AnimDelayIn);

#endif