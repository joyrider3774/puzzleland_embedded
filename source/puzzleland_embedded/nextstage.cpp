#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sound.h"
#include "helperfuncs.h"
#include "cspaceship.h"
#include "ccloud.h"
#include "cstageblock.h"
#include "cplayer.h"
#include "cfairy.h"
#include "commonvars.h"
#include "gamecommon.h"
#include "bandrender.h"
#include "nextstage.h"
	
CStageBlock *StageBlock;
CPlayer* Player;
CCloud* Cloud1;
CCloud* Cloud2;
CCloud* Cloud3;
CCloud* Cloud4;
CSpaceShip* SpaceShip;

bool BridgeShown;
bool BridgeDrawing;
int BridgeDrawnWidth;

//what the painting is told to show, worked out once a frame before the strips are put together
static bool FairyLevel = false;
static bool PasswordShown = false;
static int PasswordWidth = 0;
//1 once the player walked into the door of a room this binary has not got: the word for it is
//held on the screen instead, since that is what carries the game on in the binary that has it
static bool endOfBuild = false;


#define BridgeX 45
#define BridgeY 103
#define BridgeSpan 40
//how much of the right of the bridge image is its end. See NextStagePaint
#define BridgeEndWidth 5

#if SCREENBUFFER == 0
// ===========================================================================
// Only the places that changed
//
// Without a screen buffer this screen painted all of itself every frame, and on the slowest
// devices that is what the frame rate went on. Several things move here at once and they are
// nowhere near each other, so one rectangle around the lot of them would be the whole screen:
// where each of them was painted last is kept instead, and every frame only the place a thing
// left and the place it is in now are painted. Each of those is still painted in one pass, so
// nothing half drawn reaches the display, see bandrender.h
// ===========================================================================

//room for everything here to be in two places at once, with some to spare
#define MaxDirty 16
static int16_t dirtyX[MaxDirty], dirtyY[MaxDirty], dirtyW[MaxDirty], dirtyH[MaxDirty];
static uint8_t dirtyCount = 0;
//the whole screen instead: one that is starting, or more places than there is room for
static bool dirtyAll = true;

//where one of the things on this screen was painted last
typedef struct { int16_t X, Y, W, H; uint8_t Phase; bool Shown; } Painted;
static Painted paintedCloud[4], paintedPlayer, paintedFairy, paintedBlock, paintedShip,
               paintedBridge, paintedBox;

//the place being painted, so that what falls outside it is not drawn for nothing
static int16_t paintX, paintY, paintW, paintH;

//adds a place to paint, cut to the screen
static void MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h)
{
	if (dirtyAll)
		return;
	if (x < 0) { w = (int16_t)(w + x); x = 0; }
	if (y < 0) { h = (int16_t)(h + y); y = 0; }
	if (x + w > WINDOW_WIDTH) w = (int16_t)(WINDOW_WIDTH - x);
	if (y + h > WINDOW_HEIGHT) h = (int16_t)(WINDOW_HEIGHT - y);
	if ((w <= 0) || (h <= 0))
		return;
	//a place already down that covers this one does for both
	for (uint8_t i = 0; i < dirtyCount; i++)
		if ((dirtyX[i] <= x) && (dirtyY[i] <= y) &&
		    (dirtyX[i] + dirtyW[i] >= x + w) && (dirtyY[i] + dirtyH[i] >= y + h))
			return;
	if (dirtyCount == MaxDirty)
	{
		dirtyAll = true;
		return;
	}
	dirtyX[dirtyCount] = x;
	dirtyY[dirtyCount] = y;
	dirtyW[dirtyCount] = w;
	dirtyH[dirtyCount] = h;
	dirtyCount++;
}

//The place a thing left and the place it is in now. Something that moved a pixel is in two places
//that touch, and one rectangle around both is barely bigger than the thing itself; something that
//jumped is in two that do not, and those are marked apart
static void MarkMoved(Painted* was, bool shown, int16_t x, int16_t y, int16_t w, int16_t h,
                      uint8_t phase)
{
	//Nothing about it changed, so what the display holds for it is still right and there is no
	//reason to paint it again. The clouds drift a quarter of a pixel a frame and the figures they
	//are drawn at only change every second or third one, which is most of their painting saved.
	//Anything painted over it by something that did move draws it again by itself, see Painting
	if ((was->Shown == shown) && (!shown || ((was->X == x) && (was->Y == y) && (was->W == w) &&
	                                         (was->H == h) && (was->Phase == phase))))
		return;
	if (was->Shown && shown)
	{
		const bool meet = (was->X < x + w) && (was->X + was->W > x) &&
		                  (was->Y < y + h) && (was->Y + was->H > y);
		if (meet)
		{
			const int16_t x0 = (was->X < x) ? was->X : x;
			const int16_t y0 = (was->Y < y) ? was->Y : y;
			const int16_t x1 = (was->X + was->W > x + w) ? (int16_t)(was->X + was->W) : (int16_t)(x + w);
			const int16_t y1 = (was->Y + was->H > y + h) ? (int16_t)(was->Y + was->H) : (int16_t)(y + h);
			MarkDirty(x0, y0, (int16_t)(x1 - x0), (int16_t)(y1 - y0));
		}
		else
		{
			MarkDirty(was->X, was->Y, was->W, was->H);
			MarkDirty(x, y, w, h);
		}
	}
	else if (was->Shown)
		MarkDirty(was->X, was->Y, was->W, was->H);
	else if (shown)
		MarkDirty(x, y, w, h);
	was->Shown = shown;
	was->X = x;
	was->Y = y;
	was->W = w;
	was->H = h;
	was->Phase = phase;
}

//whether a thing reaches into the place being painted
static bool Painting(const Painted* p)
{
	return p->Shown && (p->X < paintX + paintW) && (p->X + p->W > paintX) &&
	       (p->Y < paintY + paintH) && (p->Y + p->H > paintY);
}

//nothing is known about what the display holds when a screen starts, so all of it is painted
static void ForgetScreen(void)
{
	dirtyCount = 0;
	dirtyAll = true;
	memset(paintedCloud, 0, sizeof(paintedCloud));
	memset(&paintedPlayer, 0, sizeof(paintedPlayer));
	memset(&paintedFairy, 0, sizeof(paintedFairy));
	memset(&paintedBlock, 0, sizeof(paintedBlock));
	memset(&paintedShip, 0, sizeof(paintedShip));
	memset(&paintedBridge, 0, sizeof(paintedBridge));
	memset(&paintedBox, 0, sizeof(paintedBox));
}

//one place: its background and everything that reaches into it, put together a strip at a time
static void PaintPlace(const uint8_t* background, void (*paint)(void),
                       int16_t x, int16_t y, int16_t w, int16_t h)
{
	paintX = x;
	paintY = y;
	paintW = w;
	paintH = h;
	if (BandRender_Begin(background, x, y, w, h))
	{
		while (BandRender_Next())
			paint();
	}
	else
	{
		drawImageRLEPart(x, y, x, y, w, h, background, fullScreenWidth, fullScreenHeight, false);
		paint();
	}
}

//the places that were painted this frame, which the text is printed over
static int16_t doneX[MaxDirty], doneY[MaxDirty], doneW[MaxDirty], doneH[MaxDirty];
static uint8_t doneCount = 0;
static bool doneAll = false;

//everything that changed since the last frame, or the whole screen when that is not known
static void PaintDirty(const uint8_t* background, void (*paint)(void))
{
	if (dirtyAll)
		PaintPlace(background, paint, 0, 0, fullScreenWidth, fullScreenHeight);
	else
		for (uint8_t i = 0; i < dirtyCount; i++)
			PaintPlace(background, paint, dirtyX[i], dirtyY[i], dirtyW[i], dirtyH[i]);
	//what was painted, for whatever is printed on top of it
	doneAll = dirtyAll;
	doneCount = dirtyCount;
	for (uint8_t i = 0; i < dirtyCount; i++)
	{
		doneX[i] = dirtyX[i];
		doneY[i] = dirtyY[i];
		doneW[i] = dirtyW[i];
		doneH[i] = dirtyH[i];
	}
	dirtyCount = 0;
	dirtyAll = false;
}

//whether anything was painted over the given place this frame. Text is drawn by the display
//library and cannot go into a strip, so it is printed after the strips; printing it when nothing
//under it was painted only rewrites the pixels it already put there, which on a display that is
//drawn straight to is a moment of half a letter for anyone watching
static bool PaintedOver(int16_t x, int16_t y, int16_t w, int16_t h)
{
	if (doneAll)
		return true;
	for (uint8_t i = 0; i < doneCount; i++)
		if ((doneX[i] < x + w) && (doneX[i] + doneW[i] > x) &&
		    (doneY[i] < y + h) && (doneY[i] + doneH[i] > y))
			return true;
	return false;
}
#define PRINTOVER(x, y, w, h) PaintedOver((int16_t)(x), (int16_t)(y), (int16_t)(w), (int16_t)(h))
//with a screen buffer the whole frame is put together off screen, everything is drawn every time
#define SHOWS(p) Painting(&(p))
#else
#define SHOWS(p) true
#define PRINTOVER(x, y, w, h) true
#endif

#define PlayerRightEnterBuilding 98
#define PlayerLeftEnterBuilding 17

void NextStageLevel1to35Init()
{
	endOfBuild = false;
	//the strip buffer lives for as long as this screen does, see bandrender.h
	BandRender_Init();
	//this screen paints itself and does not use needRedraw, see GameInit
	needRedraw = 1;
#if SCREENBUFFER == 0
	ForgetScreen();
#endif
	//what the painting below is told to show, put back for a screen that starts
	FairyLevel = false;
	PasswordShown = false;
	PasswordWidth = 0;

	BridgeShown = false;
	BridgeDrawing = false;
	BridgeDrawnWidth = 0;

	StageBlock = CStageBlock_Create();
	//MinX and MaxX are the walk, on a 128 pixel wide screen: the door of the building on the
	//right is at X > 99, so MaxX is just past it. The Playdate version's 30 and 268 are for its
	//400 pixel wide one and do not even fit in the uint8_t they are kept in
	Player = CPlayer_Create(15,90,15,100);
	//The password stands in a box over rows 16 to 25, and the clouds drift the whole width of the
	//screen, so any of them level with it would pass behind it: the box would cut a hole in the
	//cloud, and the figures would have to be printed again every time one went by. They are kept
	//out of that band instead, which leaves the box in clear sky. The big ones drift faster, so
	//they are the near ones and sit high; the small slow ones are the far ones and sit down by the
	//horizon, which the buildings reach at row 53. A big cloud is 14 rows and a small one 7
	Cloud1 = CCloud_Create(120,1,CLOUD_SPEED(-0.40f),Big);
	Cloud2 = CCloud_Create(90,30,CLOUD_SPEED(-0.25f),Small);
	Cloud3 = CCloud_Create(50,2,CLOUD_SPEED(-0.40f),Big);
	Cloud4 = CCloud_Create(25,42,CLOUD_SPEED(-0.25f),Small);
	SpaceShip = CSpaceShip_Create();
	Fairy = CFairy_Create(64,75,6);
	switch (Level)
	{
		case 1:
			CStageBlock_Load(StageBlock, 1);
			break;
  		case 2:
			CStageBlock_Load(StageBlock,2);
  			break;
  		case 3:
			CStageBlock_Load(StageBlock,3);
  			break;
  		case 4:
			CStageBlock_Load(StageBlock,4);
  			break;
  		case 5:
			CStageBlock_Load(StageBlock,5);
  			break;
  		case 6:
			CStageBlock_Load(StageBlock,6);
  			break;
  		case 8:
  			CStageBlock_Load(StageBlock,7);
  			break;
  		case 9:
  			CStageBlock_Load(StageBlock,8);
  			break;
  		case 11:

  			CStageBlock_Load(StageBlock,9);
  			break;
  		case 35:
  			CStageBlock_Load(StageBlock,10);
  			break;
  		default: Fairy->Hidden = true;
	}
}

void NextStageLevel1to35DeInit()
{
	BandRender_Deinit();
	CStageBlock_destroy(StageBlock);
	CSpaceShip_destroy(SpaceShip);
	CFairy_Destroy(Fairy);
	CCloud_Destroy(Cloud1);
	CCloud_Destroy(Cloud2);
	CCloud_Destroy(Cloud3);
	CCloud_Destroy(Cloud4);
	CPlayer_Destroy(Player);
}

//what the between stage screen paints, in the order it is painted. It is called once for every
//strip when the strips are used and once straight to the display when they are not, so it only
//draws and never changes anything
static void NextStagePaint(void)
{
	if (SHOWS(paintedCloud[0])) CCloud_Draw(Cloud1);
	if (SHOWS(paintedCloud[1])) CCloud_Draw(Cloud2);
	if (SHOWS(paintedCloud[2])) CCloud_Draw(Cloud3);
	if (SHOWS(paintedCloud[3])) CCloud_Draw(Cloud4);
	if ((BridgeShown || BridgeDrawing) && SHOWS(paintedBridge))
	{
		//The bridge image is bridgeWidth across but only BridgeSpan of it is ever crossed, so
		//drawing it from its left would stop in the middle of the plank and the end that was drawn
		//on the right of the image, the last BridgeEndWidth pixels with the corner taken off, would
		//never be seen. The plank is drawn up to where the bridge reaches and that end put on it
		const int16_t width = BridgeShown ? BridgeSpan : (int16_t)BridgeDrawnWidth;
		const int16_t endWidth = (width < BridgeEndWidth) ? width : BridgeEndWidth;
		//a width of 0 leaves both of these to draw nothing
		drawImageRLEPart(BridgeX, BridgeY, 0, 0, width - endWidth,
		                 bridgeHeight, ImgBridge, bridgeWidth, bridgeHeight, true);
		drawImageRLEPart(BridgeX + width - endWidth, BridgeY, bridgeWidth - endWidth, 0, endWidth,
		                 bridgeHeight, ImgBridge, bridgeWidth, bridgeHeight, true);
	}
	if (SHOWS(paintedPlayer)) CPlayer_Draw(Player);
	if (FairyLevel)
	{
		if (SHOWS(paintedFairy)) CFairy_Draw(Fairy);
		if ((Player->State == LookingUp) && SHOWS(paintedBlock))
			CStageBlock_Draw(StageBlock);
	}
	//the box the password stands in, so only the figures themselves are left to print
	if (PasswordShown && SHOWS(paintedBox))
		fillRect((WINDOW_WIDTH >> 1) - (PasswordWidth >> 1) - 2, 16, PasswordWidth + 4, 10, ColorBackground);
	if (SHOWS(paintedShip)) CSpaceShip_Draw(SpaceShip);
}

void NextStageLevel1to35()
{
	if(GameState == GSNextStageInit)
	{
		NextStageLevel1to35Init();
		GameState -= GSInitDiff;
	}
	
	if (GameState == GSNextStage)
	{
		//The room that comes next is in another binary. Its word is what carries the game on
		//there, so it is held on a screen of its own: the walk is over and running any of it
		//would leave the player standing in a door he cannot go through
		if (endOfBuild)
		{
			if (needRedraw)
			{
				fillRect(0, 0, fullScreenWidth, fullScreenHeight, ColorPaper);
				tftPrint((WINDOW_WIDTH >> 1) - ((11 * 6) >> 1), 50, "NEXT BINARY", 11,
				         ColorText, ColorText, 1);
				tftPrint((WINDOW_WIDTH >> 1) - ((4 * 6) >> 1), 70, LevelPasswords[Level], 4,
				         ColorText, ColorText, 1);
				needRedraw = 0;
			}
			if (((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A)) ||
			    ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B)) ||
			    ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R)))
				GameState = GSTitleScreenInit;
			if(GameState != GSNextStage)
				NextStageLevel1to35DeInit();
			return;
		}

		if (((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A)) ||
			((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B)) ||
			((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP)))
		{
			if(CPlayer_GetX(Player) > PlayerRightEnterBuilding)
				Player->State = EnterBuilding;
			if(CPlayer_GetX(Player) < PlayerLeftEnterBuilding)
				Player->State = EnterBuilding;
		}

		if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
		{
			GameState = GSTitleScreenInit;
		}

		//Everything that moves, once for the frame. The painting below happens once for every
		//strip of the screen, so nothing that changes the scene may sit in it
		CCloud_Move(Cloud1);
		CCloud_Move(Cloud2);
		CCloud_Move(Cloud3);
		CCloud_Move(Cloud4);

		if (!BridgeShown)
		{
			if(!BridgeDrawing)
			{
				if(CPlayer_GetX(Player) > BridgeX-ryfPlayerWidth)
				{
					BridgeDrawing = true;
					playBridgeSound();
				}
			}
			else
			{
				//the bridge grows out of the left bank until it reaches the right one
				BridgeDrawnWidth = BridgeDrawnWidth + 5;
				if (BridgeDrawnWidth >= BridgeSpan)
				{
					BridgeDrawnWidth = BridgeSpan;
					BridgeDrawing = false;
					BridgeShown = true;
				}
			}
		}
		CPlayer_Move(Player);
		FairyLevel = false;
		switch(Level)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 8:
			case 9:
			case 11:
			case 35:
				FairyLevel = true;
				CFairy_Move(Fairy);

				if((Player->State != LookingUp) && (! Fairy->Hidden) && (CPlayer_GetX(Player) + CPlayer_GetWidth(Player) >= CFairy_GetX(Fairy)))
				{
					playElfSound();
					Fairy->Hidden = true;
					Player->State = LookingUp;
				}
				if (Player->State == LookingUp)
				{
					CStageBlock_Move(StageBlock);
					if( CStageBlock_GetY(StageBlock) + CStageBlock_GetHeight(StageBlock) >= CPlayer_GetY(Player))
					{
						Player->State = Waiting;
						StageBlock->Hidden = true;
					}
				}
				break;
		}

		CSpaceShip_Move(SpaceShip);
		//The word carried past is the one for the room that comes next, which is the room the
		//building on the right is. After the last room of the game there is no next room and
		//no word for one; after the last room this build holds there is, and it is how the
		//player carries on in the binary that has it
		const bool hasNextRoom = (Level < LEVELCOUNT);
		int textw = hasNextRoom ? (int)(strlen(LevelPasswords[Level]) * 6) : 0;
		PasswordShown = hasNextRoom &&
		                (CSpaceShip_GetX(SpaceShip) > (WINDOW_WIDTH >> 1) - (textw >> 1));
		PasswordWidth = textw;

#if SCREENBUFFER == 0
		//where everything stands now against where it was painted, before any of it is painted
		MarkMoved(&paintedCloud[0], true, CCloud_ScreenX(Cloud1), Cloud1->Y, Cloud1->Width, Cloud1->Height, 0);
		MarkMoved(&paintedCloud[1], true, CCloud_ScreenX(Cloud2), Cloud2->Y, Cloud2->Width, Cloud2->Height, 0);
		MarkMoved(&paintedCloud[2], true, CCloud_ScreenX(Cloud3), Cloud3->Y, Cloud3->Width, Cloud3->Height, 0);
		MarkMoved(&paintedCloud[3], true, CCloud_ScreenX(Cloud4), Cloud4->Y, Cloud4->Width, Cloud4->Height, 0);
		//the player with the shadow under him, which reaches a little wider than he does. He walks
		//on the spot as well, so the frame he is drawn in counts as much as where he stands
		MarkMoved(&paintedPlayer, true, Player->X, Player->Y,
		          (int16_t)((ryfPlayerWidth > 1 + ryfShadowWidth) ? ryfPlayerWidth : 1 + ryfShadowWidth), ryfPlayerHeight + ryfShadowHeight,
		          Player->AnimPhase);
		MarkMoved(&paintedFairy, FairyLevel && !Fairy->Hidden,
		          (int16_t)Fairy->X, (int16_t)Fairy->Y, ryfFairyWidth, ryfFairyHeight,
		          (uint8_t)Fairy->AnimPhase);
		MarkMoved(&paintedBlock, FairyLevel && (Player->State == LookingUp) && !StageBlock->Hidden,
		          StageBlock->X, StageBlock->Y, StageBlock->Width, StageBlock->Height, 0);
		MarkMoved(&paintedBridge, BridgeShown || BridgeDrawing, BridgeX, BridgeY,
		          BridgeShown ? BridgeSpan : (int16_t)BridgeDrawnWidth, bridgeHeight, 0);
		MarkMoved(&paintedShip, true, SpaceShip->X, 11, spaceshipWidth, spaceshipHeight, 0);
		MarkMoved(&paintedBox, PasswordShown,
		          (int16_t)((WINDOW_WIDTH >> 1) - (PasswordWidth >> 1) - 2), 16,
		          (int16_t)(PasswordWidth + 4), 10, 0);
		//every place put together a strip at a time, so the display never shows the background
		//or a sprite on its own, see bandrender.h
		PaintDirty(ImgBetweenStage, NextStagePaint);
#else
		drawImageRLE(0, 0, fullScreenWidth, fullScreenHeight, ImgBetweenStage);
		NextStagePaint();
#endif

		//the text last: the font is drawn by the display library itself and cannot go into a
		//strip. It only puts figures on top of what is already right, so there is nothing to see
		if (PasswordShown && PRINTOVER((WINDOW_WIDTH >> 1) - (textw >> 1), 17, textw, 8))
			tftPrint((WINDOW_WIDTH >> 1) - (textw >> 1), 17, LevelPasswords[Level], strlen(LevelPasswords[Level]), ColorForeground, ColorBackground ,1);
        
		char ChrLevel[10];
		//the building on the right is the room that comes next, the one on the left the room
		//that was just played: walking back into it plays it again
		if (PRINTOVER(101, 60, 12, 8))
		{
			snprintf(ChrLevel,9, "%02d", Level + 1);
			tftPrint(101, 60, ChrLevel, strlen(ChrLevel), ColorText, ColorText, 1);
		}
		if (PRINTOVER(17, 60, 12, 8))
		{
			snprintf(ChrLevel,9, "%02d", Level);
			tftPrint(17, 60, ChrLevel, strlen(ChrLevel), ColorText, ColorText, 1);
		}

		if (Player->State == EnteredBuilding)
		{
			//The building on the right is room Level + 1, which is slot Level of the table, and
			//the one on the left is the room just left. A build holds a run of the rooms, so the
			//room past its last is in another binary and there is nowhere to walk on to
			if(CPlayer_GetX(Player) > PlayerRightEnterBuilding)
			{
				if (!LEVELBUILT(Level))
				{
					//The room is in another binary, and its word is what carries the game on there,
					//so it is held on a screen of its own. Past the last room of the game there is
					//no next room and no word for one, and the game is simply over
					if (Level >= LEVELCOUNT)
					{
						GameState = GSTitleScreenInit;
					}
					else
					{
						endOfBuild = true;
						needRedraw = 1;
					}
				}
				else
				{
					Level++;
					GameState = GSGameInit;
				}
			}
			else if (!LEVELBUILT(Level - 1))
			{
				GameState = GSTitleScreenInit;
			}
			else
				GameState = GSGameInit;
		}
	}
	
	if(GameState != GSNextStage)
		NextStageLevel1to35DeInit();
}

void NextStageLevel0Init()
{
	//the strip buffer lives for as long as this screen does, see bandrender.h
	BandRender_Init();
	needRedraw = 1;
#if SCREENBUFFER == 0
	ForgetScreen();
#endif
	FairyLevel = false;
	PasswordShown = false;
	PasswordWidth = 0;

	BridgeShown = false;
	BridgeDrawing = false;
	BridgeDrawnWidth = 0;

	//the first screen starts next to the building on the right, see NextStageLevel1to35Init
	Player = CPlayer_Create(80,90,80,100);
	Cloud1 = CCloud_Create(120,7,CLOUD_SPEED(-0.40f),Big);
	Cloud2 = CCloud_Create(90,20,CLOUD_SPEED(-0.25f),Small);
	Cloud3 = CCloud_Create(50,3,CLOUD_SPEED(-0.40f),Big);
	Cloud4 = CCloud_Create(25,25,CLOUD_SPEED(-0.25f),Small);;
}

void NextStageLevel0DeInit()
{
	BandRender_Deinit();
	CPlayer_Destroy(Player);
	CCloud_Destroy(Cloud1);
	CCloud_Destroy(Cloud2);
	CCloud_Destroy(Cloud3);
	CCloud_Destroy(Cloud4);
}

//the same for the first screen, which has no bridge, fairy or spaceship
static void NextStage0Paint(void)
{
	if (SHOWS(paintedCloud[0])) CCloud_Draw(Cloud1);
	if (SHOWS(paintedCloud[1])) CCloud_Draw(Cloud2);
	if (SHOWS(paintedCloud[2])) CCloud_Draw(Cloud3);
	if (SHOWS(paintedCloud[3])) CCloud_Draw(Cloud4);
	if (SHOWS(paintedPlayer)) CPlayer_Draw(Player);
}

void NextStageLevel0()
{
	if(GameState == GSNextStageInit)
	{
		NextStageLevel0Init();
		GameState -= GSInitDiff;
	}	
	
	if (((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A)) ||
		((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B)) || 
		((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP)))
	{
		if (CPlayer_GetX(Player) > PlayerRightEnterBuilding)
			Player->State = EnterBuilding;
	}

	if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
	{
		GameState = GSTitleScreenInit;
	}

	//everything that moves, once for the frame, see NextStageLevel1to35
	CCloud_Move(Cloud1);
	CCloud_Move(Cloud2);
	CCloud_Move(Cloud3);
	CCloud_Move(Cloud4);
	CPlayer_Move(Player);

#if SCREENBUFFER == 0
	//where everything stands now against where it was painted, see NextStageLevel1to35
	MarkMoved(&paintedCloud[0], true, CCloud_ScreenX(Cloud1), Cloud1->Y, Cloud1->Width, Cloud1->Height, 0);
	MarkMoved(&paintedCloud[1], true, CCloud_ScreenX(Cloud2), Cloud2->Y, Cloud2->Width, Cloud2->Height, 0);
	MarkMoved(&paintedCloud[2], true, CCloud_ScreenX(Cloud3), Cloud3->Y, Cloud3->Width, Cloud3->Height, 0);
	MarkMoved(&paintedCloud[3], true, CCloud_ScreenX(Cloud4), Cloud4->Y, Cloud4->Width, Cloud4->Height, 0);
	MarkMoved(&paintedPlayer, true, Player->X, Player->Y,
	          (int16_t)((ryfPlayerWidth > 1 + ryfShadowWidth) ? ryfPlayerWidth : 1 + ryfShadowWidth), ryfPlayerHeight + ryfShadowHeight,
	          Player->AnimPhase);
	//with BETWEENSTAGEFIRSTPICTURE off the first stage shows what every other stage shows
	PaintDirty(BETWEENSTAGEFIRSTPICTURE ? ImgBetweenStageLevel1 : ImgBetweenStage,
	           NextStage0Paint);
#else
	drawImageRLE(0, 0, fullScreenWidth, fullScreenHeight,
	             BETWEENSTAGEFIRSTPICTURE ? ImgBetweenStageLevel1 : ImgBetweenStage);
	NextStage0Paint();
#endif

	//the text last, it cannot go into a strip. Nothing of this screen reaches the number, so it
	//stands on top of the same pixels it always did
	char ChrLevel[10];
	if (PRINTOVER(102, 60, 12, 8))
	{
		snprintf(ChrLevel,9, "%02d", Level + 1);
		tftPrint(102, 60, ChrLevel, strlen(ChrLevel), ColorText, ColorText, 1);
	}
	if (Player->State == EnteredBuilding)
	{
		//There is the one building on this screen, the room the build starts at, so coming
		//in at its door is always a step on to it. The walking screen has a building either
		//side and has to ask which was entered; here there is nothing to ask
		if (!LEVELBUILT(Level))
		{
			GameState = GSTitleScreenInit;
			return;
		}
		Level++;
		GameState = GSGameInit;
	}

	if(GameState != GSNextStage)
		NextStageLevel0DeInit();
}
