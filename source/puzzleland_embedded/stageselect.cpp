#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "helperfuncs.h"
#include "sound.h"
#include "cstageselectselector.h"
#include "commonvars.h"
#include "gamecommon.h"
#include "bandrender.h"
#include "stageselect.h"


CStageSelectSelector *StageSelectSelector;

void StageSelectInit()
{
	StageSelectSelector = CStageSelectSelector_Create();
	//The cursor stands on a room this build has. They are a run of the 36, see FIRSTLEVEL
	//in defines.h, so the first of them is where it starts
	StageSelectSelector->X = (uint8_t)(FIRSTLEVEL % StageSelectCols);
	StageSelectSelector->Y = (uint8_t)(FIRSTLEVEL / StageSelectCols);
}

void StageSelectDeInit()
{
	CStageSelectSelector_destroy(StageSelectSelector);
}

//where the number of a room is printed. The arrow stands in the gap in front of it
static int16_t RoomX(uint8_t selection)
{
	return (int16_t)(XOffsetStageSelect + (selection % StageSelectCols) * StageSelectCellWidth + 7);
}

static int16_t RoomY(uint8_t selection)
{
	return (int16_t)(YOffsetStageSelect + (selection / StageSelectCols) * StageSelectCellHeight);
}

//Only the gap in front of a number, which is the whole of where the arrow stands. The numbers
//never change, only the arrow moves over them, so a move paints the gap it left and the gap it
//moved to and touches nothing else. Painting the whole screen means the paper and all thirty six
//numbers, and every number is a write of its own, which on a device without a screen buffer is a
//stall every time the cursor moves
static void PaintRoomArrow(uint8_t selection, bool withArrow)
{
	//The gap between one number and the next, which is the whole of where the arrow stands: a
	//number is two figures of 6 pixels and the cells are StageSelectCellWidth apart. Working it
	//out from the layout and not from where the arrow's image happens to start is what keeps
	//this right when the arrow is moved a pixel or two
	const int16_t gap = (int16_t)(StageSelectCellWidth - 2 * 6);
	const int16_t rx = RoomX(selection);
	const int16_t x = (int16_t)(rx - gap);
	const int16_t y = (int16_t)(RoomY(selection) - 3);
	const int16_t w = gap;
	const int16_t h = selectHeight;

#if PAPERBACKGROUND
	if (BandRender_Begin(ImgPaper, x, y, w, h))
	{
		while (BandRender_Next())
			if (withArrow)
				CStageSelectSelector_Draw(StageSelectSelector);
	}
	else
	{
		drawImageRLEPart(x, y, x, y, w, h, ImgPaper, fullScreenWidth, fullScreenHeight, false);
		if (withArrow)
			CStageSelectSelector_Draw(StageSelectSelector);
	}
#else
	//no paper to put back, so the ground it stood on is filled instead
	fillRect(x, y, w, h, ColorPaper);
	if (withArrow)
		CStageSelectSelector_Draw(StageSelectSelector);
#endif
}

void StageSelect()
{
	if(GameState == GSStageSelectInit)
	{
		StageSelectInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}

	if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
	{
		GameState = GSPasswordEntryInit;
	}

	if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
	{
		GameState = GSTitleScreenInit;
	}

	if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
	{
		//The rooms this build has not got are blank in the grid and are not entered. Their
		//slots in the lookup table are null, so going into one would be a fault
		const uint8_t room = CStageSelectSelector_GetSelection(StageSelectSelector);
		if (LEVELBUILT(room))
		{
			playMenuSelectSound();
			Level = room;
			GameState = GSOldManSpeakingInit;
		}
	}
	
	//only a selection that really moved changes the screen, see needRedraw in commonvars.h
	const uint8_t wasSelected = CStageSelectSelector_GetSelection(StageSelectSelector);

	if ((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP))
		CStageSelectSelector_MoveUp(StageSelectSelector);

	if ((currButtons & BUTTON_DOWN) && !(prevButtons & BUTTON_DOWN))
		CStageSelectSelector_MoveDown(StageSelectSelector);

	if ((currButtons & BUTTON_LEFT) && !(prevButtons & BUTTON_LEFT))
		CStageSelectSelector_MoveLeft(StageSelectSelector);

	if ((currButtons & BUTTON_RIGHT) && !(prevButtons & BUTTON_RIGHT))
		CStageSelectSelector_MoveRight(StageSelectSelector);

	uint8_t nowSelected = CStageSelectSelector_GetSelection(StageSelectSelector);
	//a move onto a room another binary holds is undone, so the cursor never stands on one
	if (!LEVELBUILT(nowSelected))
	{
		StageSelectSelector->X = (uint8_t)(wasSelected % StageSelectCols);
		StageSelectSelector->Y = (uint8_t)(wasSelected / StageSelectCols);
		nowSelected = wasSelected;
	}

	if (needRedraw)
	{
			#if PAPERBACKGROUND
		drawImageRLE(0, 0, fullScreenWidth, fullScreenHeight, ImgPaper);
		#else
		fillRect(0, 0, fullScreenWidth, fullScreenHeight, ColorPaper);
		#endif
		tftPrint((WINDOW_WIDTH >> 1) - ((strlen("ROOM SELECT") * 6) >> 1), 8, "ROOM SELECT", strlen("ROOM SELECT"), ColorText,ColorText,1);

		int Teller;
		char ChrRoom[10];

		//the 36 rooms in a grid of StageSelectCols by StageSelectRows, the number of every one of
		//them a cell width apart and far enough in for the selector arrow to stand in front of it
		for (Teller=0;Teller< StageSelectCols * StageSelectRows;Teller++)
		{
			//a room another build holds is left blank, so what is here is plain to see
			if (!LEVELBUILT(Teller))
				continue;
			snprintf(ChrRoom,9,"%2d",Teller+1);
			tftPrint(XOffsetStageSelect + (Teller % StageSelectCols) * StageSelectCellWidth + 7,
			         YOffsetStageSelect + (Teller / StageSelectCols) * StageSelectCellHeight, ChrRoom, strlen(ChrRoom), ColorText,ColorText,1);
		}
		CStageSelectSelector_Draw(StageSelectSelector);
		needRedraw = 0;
	}
	else if (nowSelected != wasSelected)
	{
		//only the gap the arrow left and the gap it moved to
		PaintRoomArrow(wasSelected, false);
		PaintRoomArrow(nowSelected, true);
	}

	if(GameState != GSStageSelect)
		StageSelectDeInit();
}
