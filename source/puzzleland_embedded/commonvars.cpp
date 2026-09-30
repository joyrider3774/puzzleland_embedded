#include <stdint.h>
#include "commonvars.h"
#include "chand.h"
#include "cfairy.h"

//audio
const char* const LevelPasswords[36] = {"DAVY","DISK","TREE","SEAL",
									 "OPEN","LION","WINE","GROW",
									 "FOOD","DUEL","MEAT","FLEE",
									 "DUST","LOST","HELP","MILK",
									 "SAFE","DULL","SAVE","BEER",
									 "BILL","BLOW","BIKE","VENT",
									 "ROOM","HOOD","MOOD","FEAR",
									 "REEF","POLE","BEEF","SONG",
									 "HOPE","BLUE","MAIL","MONK"};
const char Letters[4][7] = {{'A','B','C','D','E','F','G'},
							  {'H','I','J','K','L','M','N'},
							  {'O','P','Q','R','S','T','U'},
							  {'V','W','X','Y','Z','0','0'}};

uint8_t prevButtons, currButtons;
uint8_t needRedraw = 1;
bool debugMode = false;
int GameMoveCoolDown, PasswordEntryCoolDown;

//common stuff
const uint8_t* Background = NULL;
const uint8_t* Text = NULL;
CFairy* Fairy;

//Game Stuff
int GameState = GSIntroInit;
int Level;
CHand* Hand;
uint32_t OldTime = 0, StartTime=0, EndTime = 0;
bool NeedGameReset = false;

//Puzzle Game Stuff
const uint8_t* BorderImages[BorderCount];
const uint8_t *ImgStageBlock1, *ImgStageBlock2, *ImgStageBlock3, *ImgStageBlock4, *ImgStageBlock5, 
              *ImgStageBlock6, *ImgStageBlock7, *ImgStageBlock8, *ImgStageBlock9, *ImgStageBlock10, 
              *ImgRyfCloud, *ImgRyfSmallCloud, *ImgRyfFairy, *ImgSelect, *ImgHand, *ImgOptionSelect,
              *ImgShadow, *ImgPlayer, *ImgSpaceship, *ImgTitleSelector, *ImgPaper, 
              *ImgRoomBackground, *ImgBlockActiveImage, *ImgBlockImage, *ImgIntro, *ImgBetweenStage, *ImgBetweenStageLevel1,
              *ImgBridge, *ImgOldMan, *ImgStageClearKader, *ImgTitle;
int PlayField[2][Cols][Rows];
bool BlockActive;

uint16_t ColorBackground, ColorForeground, ColorText;
//the sheet the menus are written on, which is white whatever the skin: without the
//paper picture the menus are filled with this and written on in ColorText
uint16_t ColorPaper;

//stage Clear Stuff
const uint8_t* RoomBackground = NULL;
const uint8_t* StageClearKader = NULL;

//Next Stage Stuff
const uint8_t* Bridge = NULL;
const uint8_t* PrevLevel = NULL;
const uint8_t* NextLevel = NULL;



