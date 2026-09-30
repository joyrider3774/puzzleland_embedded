#ifndef CPASSWORDSELECTOR_H
#define CPASSWORDSELECTOR_H

#include <stdint.h>

typedef struct CPasswordSelector CPasswordSelector;
struct CPasswordSelector
{
	uint8_t Selection, X, Y;
} ;

void CPasswordSelector_Destroy(CPasswordSelector* Selector);
uint8_t CPasswordSelector_GetY(CPasswordSelector* Selector);
uint8_t CPasswordSelector_GetX(CPasswordSelector* Selector);
void CPasswordSelector_MoveUp(CPasswordSelector* Selector);
void CPasswordSelector_MoveRight(CPasswordSelector* Selector);
void CPasswordSelector_MoveLeft(CPasswordSelector* Selector);
void CPasswordSelector_MoveDown(CPasswordSelector* Selector);
void CPasswordSelector_Draw(CPasswordSelector* Selector);
CPasswordSelector* CPasswordSelector_Create();

#endif