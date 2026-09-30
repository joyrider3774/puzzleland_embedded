#ifndef CSTAGESELECTSELECTOR_H
#define CSTAGESELECTSELECTOR_H

#include <stdint.h>

typedef struct CStageSelectSelector CStageSelectSelector;
struct CStageSelectSelector
{
	uint8_t X,Y;
};

CStageSelectSelector* CStageSelectSelector_Create();
void CStageSelectSelector_Draw(CStageSelectSelector* selector);
void CStageSelectSelector_MoveDown(CStageSelectSelector* selector);
void CStageSelectSelector_MoveLeft(CStageSelectSelector* selector);
void CStageSelectSelector_MoveRight(CStageSelectSelector* selector);
void CStageSelectSelector_MoveUp(CStageSelectSelector* selector);
uint8_t CStageSelectSelector_GetSelection(CStageSelectSelector* selector);
void CStageSelectSelector_destroy(CStageSelectSelector* selector);

#endif