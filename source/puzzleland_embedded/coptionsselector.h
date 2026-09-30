#ifndef COPTIONSSELECTOR_H
#define COPTIONSSELECTOR_H

#include <stdint.h>

typedef struct COptionsSelector COptionsSelector;
struct COptionsSelector
{
	uint8_t Selection;
};

void COptionsSelector_Destroy(COptionsSelector* Selector);
void COptionsSelector_MoveUp(COptionsSelector* Selector);
void COptionsSelector_MoveDown(COptionsSelector* Selector);
uint8_t COptionsSelector_GetSelection(COptionsSelector* Selector);
void COptionsSelector_Draw(COptionsSelector* Selector);
COptionsSelector* COptionsSelector_Create();

#endif