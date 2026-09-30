#ifndef CSTAGEBLOCK_H
#define CSTAGEBLOCK_H

#include <stdint.h>

typedef struct CStageBlock CStageBlock;
struct CStageBlock
{
 	bool Hidden;
 	int16_t Y;
	uint8_t X,Yi,Width,Height;
 	const uint8_t* Image;
};

void CStageBlock_destroy(CStageBlock* stageBlock);
uint8_t CStageBlock_GetHeight(CStageBlock* stageBlock);
int16_t CStageBlock_GetY(CStageBlock* stageBlock);
void CStageBlock_Move(CStageBlock* stageBlock);
void CStageBlock_Draw(CStageBlock* stageBlock);
void CStageBlock_Load(CStageBlock* stageBlock, const uint8_t BlockNr);
CStageBlock* CStageBlock_Create();

#endif