#ifndef COMMONVARS_H
#define COMMONVARS_H

#include <stdint.h>
#include "defines.h"
#include "Platform.h"
#include "chand.h"
#include "cfairy.h"


extern const char* const LevelPasswords[36];
extern const char Letters[4][7];

//common stuff
extern const uint8_t *ImgStageBlock1, *ImgStageBlock2, *ImgStageBlock3, *ImgStageBlock4, *ImgStageBlock5, 
                     *ImgStageBlock6, *ImgStageBlock7, *ImgStageBlock8, *ImgStageBlock9, *ImgStageBlock10, 
                     *ImgRyfCloud, *ImgRyfSmallCloud, *ImgRyfFairy, *ImgSelect, *ImgHand, *ImgOptionSelect,
                     *ImgShadow, *ImgPlayer, *ImgSpaceship, *ImgTitleSelector, *ImgPaper, 
                     *ImgRoomBackground, *ImgBlockActiveImage, *ImgBlockImage, *ImgIntro, *ImgBetweenStage, *ImgBetweenStageLevel1,
                     *ImgBridge, *ImgOldMan, *ImgStageClearKader, *ImgTitle;

extern CFairy* Fairy;
extern uint16_t ColorBackground, ColorForeground, ColorText;
//the colour of the sheet the menus are written on, see PAPERBACKGROUND
extern uint16_t ColorPaper;

//1 = something on the screen changed and it has to be painted again. The screens that stand
//still (the menus, the credits, the room selector) only paint when this is set: with
//SCREENBUFFER 0 every drawing call goes straight to the display, so painting a screen that
//did not change is the background wiping the sprites off it and putting them back, which is
//what makes a device like the Gamebuino META flicker. A screen sets it when it is entered and
//whenever anything it draws moves, and clears it once it has painted
extern uint8_t needRedraw;

//Game Stuff
extern int GameState;
extern int Level;
extern CHand* Hand;
extern uint32_t OldTime, StartTime, EndTime;
extern int GameMoveCoolDown, PasswordEntryCoolDown;
extern bool NeedGameReset;

//Puzzle Game Stuff
extern const uint8_t *BorderImages[BorderCount];
extern int PlayField[2][Cols][Rows];
extern bool BlockActive;

//stage Clear Stuff
extern const uint8_t* RoomBackground,*StageClearKader;

//Next Stage Stuff
extern const uint8_t* Bridge;
extern const uint8_t* PrevLevel;
extern const uint8_t* NextLevel;
extern uint8_t currButtons, prevButtons;
#endif