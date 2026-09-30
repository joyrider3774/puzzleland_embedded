#ifndef CTITLESCREENSELECTOR_H
#define CTITLESCREENSELECTOR_H

#include <stdint.h>

typedef struct CTitleScreenSelector CTitleScreenSelector;
struct CTitleScreenSelector
{
	uint8_t X,Selection;
};

CTitleScreenSelector* CTitleScreenSelector_Create();
void CTitleScreenSelector_MoveUp(CTitleScreenSelector* selector);
void CTitleScreenSelector_MoveDown(CTitleScreenSelector* selector);
void CTitleScreenSelector_Draw(CTitleScreenSelector* selector);
void CTitleScreenSelector_destroy(CTitleScreenSelector* selector);

#endif