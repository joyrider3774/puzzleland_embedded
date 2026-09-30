#ifndef SAVESTATE_H
#define SAVESTATE_H

#include <stdint.h>

//reads what was saved, or starts from the defaults when there is nothing to trust
void initSaveState(void);
//sound and music together, they are the only options the game has
void setOptionsSaveState(uint8_t soundValue, uint8_t musicValue);
uint8_t isSoundOnSaveState(void);
uint8_t isMusicOnSaveState(void);

#endif
