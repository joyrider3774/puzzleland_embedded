#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "sound.h"
#include "levels.h"
#include "gamecommon.h"
#include "bandrender.h"
#include "game.h"

void MoveBlockRight()
{
	int X,Y;
	bool CanMove;
	CanMove = true;
	for (Y=0;Y<Rows;Y++)
		if(PlayField[1][Cols-1][Y] != 0)
		{
			CanMove = false;
			break;
		}
	if (CanMove)
		for(X=Cols-1;X >= 1;X--)
			for(Y=0;Y<Rows;Y++)
				PlayField[1][X][Y] = PlayField[1][X-1][Y];
	for (Y=0;Y<Rows;Y++)
		PlayField[1][0][Y] = 0;
}

void MoveBlockLeft()
{
	int X,Y;
	bool CanMove;
	CanMove = true;
	for (Y=0;Y<Rows;Y++)
		if(PlayField[1][0][Y] != 0)
		{
			CanMove = false;
			break;
		}
	if (CanMove)
		for(X=0;X <Cols-1;X++)
			for(Y=0;Y<Rows;Y++)
				PlayField[1][X][Y] = PlayField[1][X+1][Y];
	for (Y=0;Y<Rows;Y++)
		PlayField[1][Cols-1][Y] = 0;
}

void MoveBlockUp()
{
	int X,Y;
	bool CanMove;
	CanMove = true;
	for (X=0;X<Cols;X++)
		if(PlayField[1][X][0] != 0)
		{
			CanMove = false;
			break;
		}
	if (CanMove)
		for(X=0;X<Cols;X++)
			for(Y=0;Y<Rows-1;Y++)
				PlayField[1][X][Y] = PlayField[1][X][Y+1];
	for (X=0;X<Cols;X++)
		PlayField[1][X][Rows-1] = 0;
}

void MoveBlockDown()
{
	int X,Y;
	bool CanMove;
	CanMove = true;
	for (X=0;X<Cols;X++)
		if(PlayField[1][X][Rows-1] != 0)
		{
			CanMove = false;
			break;
		}
	if (CanMove)
		for(X=0;X<Cols;X++)
			for(Y=Rows-1;Y>=1;Y--)
				PlayField[1][X][Y] = PlayField[1][X][Y-1];
	for (X=0;X<Cols;X++)
		PlayField[1][X][0] = 0;
}

void MakeBlockActive(const int X,const int Y,const int BlockNr)
{
	int TellerX,TellerY;
	if((X >= 0) && (Y >=0) && (X < Cols) && (Y < Rows))
	{

		for(TellerX=Cols-1;TellerX>=0;TellerX--)
			for(TellerY=Rows-1;TellerY>=0;TellerY--)
			{
				if(PlayField[0][TellerX][TellerY] == BlockNr)
				{
					PlayField[0][TellerX][TellerY] = 0;
					PlayField[1][TellerX][TellerY] = BlockNr;
				}
			}
	}
}

bool MakeBlockUnActive()
{
	int X,Y;
	bool FirstBlockFound,UnActive;
	FirstBlockFound = false;
	UnActive = true;
	for (X=Cols-1;X>=0;X--)
		for(Y=Rows-1;Y>=0;Y--)
			if ((PlayField[1][X][Y] > 0)&& (PlayField[0][X][Y] != 0))
			{
				UnActive = false;
				break;
			}
	if(UnActive)
		for(X=Cols-1;X >=0; X--)
			for(Y=Rows-1 ; Y >=0 ;Y--)
				if(PlayField[1][X][Y] > 0)
				{
					if (!FirstBlockFound)
					{
						CHand_SetPosition(Hand,X, Y );
						FirstBlockFound = true;
					}
					PlayField[0][X][Y] = PlayField[1][X][Y];
					PlayField[1][X][Y] = 0;
				}
	return UnActive;
}

void FlipBlock(const bool Horizontal)
{
	int BlockW,BlockH,X,Y,MinX,MinY,MaxX,MaxY;
	int Tmp[Cols][Rows];
	MinX = 100;
	MinY = 100;
	MaxX = -1;
	MaxY = -1;
	for (X = Cols-1 ; X >=0; X--)
		for(Y = Rows-1; Y>=0;Y--)
		{
			if(PlayField[1][X][Y] > 0)
			{
				if(MinX > X)
					MinX = X;
				if(MaxX < X)
					MaxX = X;
				if(MinY > Y)
					MinY = Y;
				if(MaxY < Y)
					MaxY = Y;
				Tmp[X][Y] = 0;
			}
		}

	BlockW = MaxX - MinX;
	BlockH = MaxY - MinY;
	for(X=BlockW;X>=0;X--)
		for(Y=BlockH;Y>=0;Y--)
		{
			if(Horizontal)
				Tmp[X][BlockH- Y] = PlayField[1][MinX + X][MinY + Y];
			else
				Tmp[BlockW - X][Y] = PlayField[1][MinX+X][MinY + Y];
			PlayField[1][MinX+X][MinY+Y] = 0;
		}
	for(X=BlockW;X>=0;X--)
		for(Y=BlockH;Y>=0;Y--)
			PlayField[1][MinX+X][MinY+Y] = Tmp[X][Y];
	playFlipBlockSound();
}

void RotateBlock()
{
 	int Tmp1,Tmp2,Offset,NewBlockW,NewBlockH,BlockW,BlockH,MinX,MinY,MaxX,MaxY,X,Y;
 	int Tmp[Cols][Rows];
	MinX = 100;
	MinY = 100;
	MaxX = -1;
	MaxY = -1;
	for (X = Cols-1 ; X >=0; X--)
		for(Y = Rows-1; Y>=0;Y--)
		{
			if(PlayField[1][X][Y] > 0)
			{
				if(MinX > X)
					MinX = X;
				if(MaxX < X)
					MaxX = X;
				if(MinY > Y)
					MinY = Y;
				if(MaxY < Y)
					MaxY = Y;
			}
			Tmp[X][Y] = 0;
		}

	BlockW = MaxX - MinX +1;
	BlockH = MaxY - MinY +1;
	if(BlockW == BlockH)
 	{
 		for(X=BlockW-1;X>=0;X--)
 			for(Y=BlockH-1;Y>=0;Y--)
 			{
 				Tmp[BlockW-1-Y][X] = PlayField[1][X+MinX][Y+MinY];
 				PlayField[1][X+MinX][Y+MinY] = 0;
 			}
 		for(X=BlockW-1;X>=0;X--)
 			for(Y=BlockH-1;Y>=0;Y--)
 				PlayField[1][MinX+X][MinY+Y] = Tmp[X][Y];

 	}

 	if(BlockW > BlockH)
 	{
  		NewBlockH = BlockW;
 
		if((MinY + ( NewBlockH / 2) >= Rows) || (MinY - ((NewBlockH -1)/ 2) <0) || (MinX + BlockW / 2 > Cols))
 		{
			playErrorSound();
 			return;
 		}

 		for(X=BlockW-1;X>=0;X--)
 		{
 			for(Y=BlockH-1;Y>=0;Y--)
 			{
 				Tmp[BlockW-1-Y][X] = PlayField[1][X+MinX][Y+MinY];
 				PlayField[1][X+MinX][Y+MinY] = 0;
 			}
 		}
 
		Tmp1 = (BlockW - 1) / 2;
 		Tmp2 = (NewBlockH-1)/ 2;

 		for(X=BlockW-1;X>=0;X--)
 			for(Y=NewBlockH-1;Y>=0;Y--)
 			{
				if ((MinX + X - Tmp1 >= 0) && (MinX + X - Tmp1 <= Cols - 1) &&
					(MinY + Y - Tmp2 >= 0) && (MinY + Y - Tmp2 <= Rows - 1))
				{
					PlayField[1][MinX + X - Tmp1][MinY + Y - Tmp2] = Tmp[X][Y];
				}
			}
 	}
 	if(BlockW < BlockH)
 	{
 		NewBlockW = BlockH;
 		NewBlockH = BlockW;

		if ((BlockW == 2) && (BlockH == 3))
			Offset = 1;
		else
			Offset = 0;

		if((MinX + Offset +  NewBlockW / 2 >= Cols) || (MinX + Offset - (NewBlockW-1)/2 <0) || (MinY + (BlockH) /2 > Rows))
		{
			playErrorSound();
 			return;
		}

		for(X=BlockW-1;X>=0;X--)
			for(Y=BlockH-1;Y>=0;Y--)
			{
				Tmp[NewBlockW-1-Y][X] = PlayField[1][X+MinX][Y+MinY];
				PlayField[1][X+MinX][Y+MinY] = 0;
			}

		Tmp1 = (NewBlockW-1) / 2;
		Tmp2 = (BlockH -1) / 2;
		for(X=NewBlockW-1;X>=0;X--)
			for(Y=NewBlockH-1;Y>=0;Y--)
			{
				if((MinX+X + Offset - Tmp1  >= 0) && (MinX+X + Offset - Tmp1 <= Cols-1) &&
				   (MinY+Y + Tmp2  >=0) && (MinY+Y + Tmp2 <= Rows-1))
				   	PlayField[1][MinX+X + Offset - Tmp1][MinY + Y +  Tmp2] = Tmp[X][Y];
			}
	}
	playRotateBlockSound();
}


//A level is stored run length encoded, see tools/convert_levels.py: a control byte with its top
//bit set stands for (c & 0x7F) + 1 copies of the byte that follows it, and one without for the
//(c + 1) bytes that follow. Most of a level is the same few block types repeated, which takes
//about 38% off it.
//A level is read in the order it is stored, a column at a time and within a column top to bottom,
//so it is decoded as it is read and none of it is held in ram
typedef struct LevelReader LevelReader;
struct LevelReader
{
	const uint8_t* pos;
	uint8_t left;            //how many block types the run or the literal still owes
	uint8_t repeated;        //the type a run repeats
	bool inRun;
};

static void LevelReaderInit(LevelReader* reader, const uint8_t* level)
{
	reader->pos = level;
	reader->left = 0;
	reader->repeated = 0;
	reader->inRun = false;
}

static int8_t LevelReaderNext(LevelReader* reader)
{
	if (reader->left == 0)
	{
		//PLATFORM_READ_BYTE is pgm_read_byte on some of the devices, which may look at what it
		//is handed more than once, so the pointer is never stepped on inside it
		uint8_t control = (uint8_t)PLATFORM_READ_BYTE(reader->pos);
		reader->pos++;
		if (control & 0x80)
		{
			reader->inRun = true;
			reader->left = (uint8_t)((control & 0x7F) + 1);
			reader->repeated = (uint8_t)PLATFORM_READ_BYTE(reader->pos);
			reader->pos++;
		}
		else
		{
			reader->inRun = false;
			reader->left = (uint8_t)(control + 1);
		}
	}
	reader->left--;
	if (reader->inRun)
		return (int8_t)reader->repeated;
	const uint8_t value = (uint8_t)PLATFORM_READ_BYTE(reader->pos);
	reader->pos++;
	return (int8_t)value;
}

void LoadLevel()
{
	int X,Y;
	//Level counts from 1, the way the level%d.lev file names did
	LevelReader reader;
	LevelReaderInit(&reader, level_data_files[0][Level - 1]);
	for (X=0;X<Cols;X++)
	{ 	for (Y=0;Y <Rows;Y++)
		{
			//Levels are stored column by column, so PlayField[0][X][Y] is the [X * Rows + Y]th
			//block type, and this reads them in that order: the encoded level gives them up one
			//after another and cannot be indexed into
			PlayField[0][X][Y] = LevelReaderNext(&reader);
			PlayField[1][X][Y] = 0;
		}
	}
}

bool IsStageClear()
{
	int X,Y,TellerX,TellerY;
	bool StartBorderFound,VertStartBorderFound,VertEndBorderFound,Temp;
	Temp = true;
	if (Level == 17)
	{
		for(Y=6;Y<=8;Y++)
			for(X=0;X<Cols;X++)
				Temp = Temp && (PlayField[0][X][Y] > 0);
	}
	else
	if (Level == 32)
	{
		for (Y=0;Y<=14;Y++)
			for (X=7;X<=9;X++)
				Temp = Temp && (PlayField[0][X][Y] > 0);
		for (Y=6;Y<=8;Y++)
			for (X = 10;X<=14;X++)
				Temp = Temp && (PlayField[0][X][Y] > 0);

	}
	else
	{
		StartBorderFound = false;
		for (Y=0;Y<Rows;Y++)
			for (X=0;X<Cols;X++)
			{
				if (!StartBorderFound)
				{
					if(X < Cols-1)
						if((PlayField[0][X][Y] < 0) && (PlayField[0][X+1][Y] >=0))
							for(TellerX = X+1;TellerX<Cols;TellerX++)
								if(PlayField[0][TellerX][Y] < 0)
									StartBorderFound = true;
				}
				else
				{
					if(PlayField[0][X][Y] < 0)
					{
						StartBorderFound = false;
						if( X < Cols-1)
							if((PlayField[0][X][Y] < 0) && (PlayField[0][X+1][Y] >= 0))
								for(TellerX = X+1;TellerX < Cols;TellerX++)
									if(PlayField[0][TellerX][Y] < 0)
										StartBorderFound=true;
						continue;
					}

					if (StartBorderFound)
					{
						VertEndBorderFound = false;
						VertStartBorderFound = false;
						if (Y < Rows -1)
							for (TellerY=Y+1;TellerY<Rows;TellerY++)
								if(PlayField[0][X][TellerY] < 0)
									VertEndBorderFound = true;
						if (Y > 0)
							for(TellerY = Y -1; TellerY >=0;TellerY--)
								if(PlayField[0][X][TellerY] < 0)
									VertStartBorderFound = true;
						if(VertStartBorderFound && VertEndBorderFound)
							Temp = Temp && (PlayField[0][X][Y] > 0);
					}

				}
			}


	}
	return Temp;
}

// ===========================================================================
// What has to be painted again
//
// Without a screen buffer every drawing call goes straight to the display, so painting the
// whole screen for a frame in which nothing moved is the background wiping the blocks off it
// and putting them back, which is what makes a device like the Gamebuino META flicker. And
// most frames change nothing at all: the hand stands still, no piece is being carried and
// only the seconds of the clock move on.
//
// So the screen keeps one rectangle of everything that changed since it was last painted. A
// frame that changed nothing paints nothing, and a frame that did paints that rectangle only:
// the piece of the background under it, the blocks that reach into it, the clock when it is
// in it and the hand on top. With a screen buffer none of this is needed, the whole frame is
// composed off screen and sent in one go, so there the board is drawn again every frame
// ===========================================================================

#if SCREENBUFFER == 0

//x1 and y1 are one past the rectangle, dirtyAny says whether there is one at all
static int16_t dirtyX0, dirtyY0, dirtyX1, dirtyY1;
static bool dirtyAny = false;

static void MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h)
{
	int16_t x1 = x + w, y1 = y + h;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x1 > WINDOW_WIDTH) x1 = WINDOW_WIDTH;
	if (y1 > WINDOW_HEIGHT) y1 = WINDOW_HEIGHT;
	if ((x >= x1) || (y >= y1))
		return;
	if (!dirtyAny)
	{
		dirtyX0 = x; dirtyY0 = y; dirtyX1 = x1; dirtyY1 = y1;
		dirtyAny = true;
		return;
	}
	if (x < dirtyX0) dirtyX0 = x;
	if (y < dirtyY0) dirtyY0 = y;
	if (x1 > dirtyX1) dirtyX1 = x1;
	if (y1 > dirtyY1) dirtyY1 = y1;
}

//the square the hand stands on at the given place on the board, the one CHand_Draw paints
//The top line as it stands on the screen. It is painted a character at a time against this, so
//"Time: " and the figures that did not move on are left alone and a second going by costs the
//one cell of the figure that changed. It is kept apart from the board's rectangle as well:
//taken together with it, every second would stretch a repaint from the top of the screen down
//to wherever the hand is and paint everything in between for nothing
static char shownLevelText[PanelTextMax] = "";
static char shownTimeText[PanelTextMax] = "";

static void MarkHandAt(int16_t handX, int16_t handY)
{
	MarkDirty(MinPlayAreaX + handX * BlockWidth, MinPlayAreaY + handY * BlockHeight,
	          handWidth, handHeight);
}

//the square around the piece being carried, which is what layer 1 of the board holds. The
//outline of a shape runs a pixel past the block on the right and the bottom. False when no
//piece is being carried, and then the square is not filled in
static bool ActiveBlockBounds(int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1)
{
	bool any = false;
	for (int X = 0; X < Cols; X++)
		for (int Y = 0; Y < Rows; Y++)
			if (PlayField[1][X][Y] != 0)
			{
				const int16_t bx = (int16_t)(XOffsetGame + X * BlockWidth);
				const int16_t by = (int16_t)(YOffsetGame + Y * BlockHeight);
				if (!any)
				{
					*x0 = bx; *y0 = by;
					*x1 = (int16_t)(bx + BlockWidth + 1); *y1 = (int16_t)(by + BlockHeight + 1);
					any = true;
				}
				else
				{
					if (bx < *x0) *x0 = bx;
					if (by < *y0) *y0 = by;
					if (bx + BlockWidth + 1 > *x1) *x1 = (int16_t)(bx + BlockWidth + 1);
					if (by + BlockHeight + 1 > *y1) *y1 = (int16_t)(by + BlockHeight + 1);
				}
			}
	return any;
}

//Where the hand and the carried piece stood when the screen was last painted. Marking is done
//against these and not every frame: the hand standing still and a piece being held still are
//not changes, and marking them anyway would repaint their square thirty times a second for
//nothing, which is the flicker this is here to stop
static int16_t paintedHandX = -1, paintedHandY = -1;
static bool paintedHandHidden = true;
static int16_t paintedPieceX0, paintedPieceY0, paintedPieceX1, paintedPieceY1;
static bool paintedPieceAny = false;
#endif

#if SCREENBUFFER == 0
//Forgets everything that says what the screen holds and asks for the whole of it again. A room
//that starts and a room that is put back to how it began both need this: the board is painted
//over completely and that takes the top line with it.
//The two texts have to be emptied whole and not just their first character, or the comparison in
//PaintPanelText walks into what was left in them and leaves those characters unpainted, which is
//what showed a clock with its "Time: " missing
static void ForgetScreen(void)
{
	dirtyAny = false;
	paintedHandX = -1;
	paintedHandY = -1;
	paintedHandHidden = true;
	paintedPieceAny = false;
	memset(shownLevelText, 0, sizeof(shownLevelText));
	memset(shownTimeText, 0, sizeof(shownTimeText));
	MarkDirty(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}
#endif

void GameInit()
{
	//the strip buffer lives for as long as the game screen does, see bandrender.h
	BandRender_Init();
#if SCREENBUFFER == 0
	//this screen paints itself and does not use needRedraw, so it leaves it set: whatever screen
	//comes after it then paints itself whole instead of inheriting a stale one from before
	needRedraw = 1;
	ForgetScreen();
#endif
	GameMoveCoolDown = 0;
	BlockActive = false;
	LoadLevel();
	CHand_SetPosition(Hand,10, 8);
	CHand_Show(Hand);
	srand (Platform_Micros());
	StartTime = Platform_Millis();
}

void GameDeInit()
{
	BandRender_Deinit();
}

#if SCREENBUFFER == 0
//Writes the characters of the text that differ from the ones on the screen, each in its own
//cell: the background of that cell and then the character. Of "Time: 00:00:07" that is usually
//the one figure of the seconds, six pixels wide
static void PaintPanelText(int16_t textX, const char* text, char* shown)
{
	for (int8_t i = 0; (text[i] != 0) || (shown[i] != 0); i++)
	{
		if (text[i] == shown[i])
			continue;
		const int16_t cellX = (int16_t)(textX + i * PanelCharW);
		//just the background of the cell, nothing of the board reaches up here
		if (BandRender_Begin(ImgRoomBackground, cellX, PanelY, PanelCharW, PanelH))
		{
			while (BandRender_Next())
				;
		}
		else
			drawImageRLEPart(cellX, PanelY, cellX, PanelY, PanelCharW, PanelH, ImgRoomBackground,
			                 fullScreenWidth, fullScreenHeight, false);
		//a character that is gone leaves the cell as the background, nothing is drawn over it
		if (text[i] != 0)
			DrawPanelChar(textX, i, text[i]);
		shown[i] = text[i];
	}
}
#endif

void Game()
{
	if(GameState == GSGameInit)
	{
		GameInit();
		GameState -= GSInitDiff;

	}

#if SCREENBUFFER == 0
	//set by the buttons below that change the board, so a frame that changed nothing marks
	//nothing and paints nothing
	bool boardMoved = false;
#endif

	if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
	{
		if (!BlockActive)
		{
			int PlayFieldX, PlayFieldY, BlockNr;
			PlayFieldX = CHand_GetPlayFieldX(Hand);
			PlayFieldY = CHand_GetPlayFieldY(Hand);
			BlockNr = PlayField[0][PlayFieldX][PlayFieldY];
			if (BlockNr > 0)
			{
				playPickupBlockSound();
				BlockActive = true;
				CHand_Hide(Hand);
				MakeBlockActive(PlayFieldX, PlayFieldY, BlockNr);
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
			}
		}
		else
		{
			if (MakeBlockUnActive())
			{
				playDropBlockSound();
				BlockActive = false;
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
				if (IsStageClear())
				{
					GameState = GSStageClearInit;
					playStageEndSound();
				}
				else
					CHand_Show(Hand);
			}
			else
				playErrorSound();
		}
	}

	if ((currButtons & BUTTON_B) && ((currButtons & BUTTON_LEFT) &&  !(prevButtons & BUTTON_LEFT)))
	{
		if (BlockActive)
		{
				FlipBlock(true);
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
		}
	}
	
	if ((currButtons & BUTTON_B) && ((currButtons & BUTTON_RIGHT) && !(prevButtons & BUTTON_RIGHT)))
	{
		if (BlockActive)
		{
			FlipBlock(false);
#if SCREENBUFFER == 0
			boardMoved = true;
#endif
		}
	}

	if ((currButtons & BUTTON_B) && ((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP)))
	{
		if (BlockActive)
		{
			RotateBlock();
#if SCREENBUFFER == 0
			boardMoved = true;
#endif
		}
	}

	if ((currButtons & BUTTON_L) && !(prevButtons & BUTTON_L))
	{
		NeedGameReset = true;
		GameMoveCoolDown = 3;
	}

	if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
	{
		GameState = GSTitleScreenInit;
	}

	if (GameMoveCoolDown > 0)
		GameMoveCoolDown--;
	if (BlockActive && (GameMoveCoolDown == 0))
	{
		if (!(currButtons & BUTTON_B))
		{
			if (currButtons & BUTTON_RIGHT)
			{
				MoveBlockRight();
				GameMoveCoolDown = 3;
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
			}
			if (currButtons & BUTTON_LEFT)
			{
				MoveBlockLeft();
				GameMoveCoolDown = 3;
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
			}
			if (currButtons & BUTTON_UP)
			{
				MoveBlockUp();
				GameMoveCoolDown = 3;
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
			}
			if (currButtons & BUTTON_DOWN)
			{
				MoveBlockDown();
				GameMoveCoolDown = 3;
#if SCREENBUFFER == 0
				boardMoved = true;
#endif
			}
		}
	}

	//set by meny callback
	if (NeedGameReset)
	{
		LoadLevel();
		BlockActive = false;
		CHand_SetPosition(Hand, 10, 8);
		CHand_Show(Hand);
		NeedGameReset = false;
#if SCREENBUFFER == 0
		//another board, so the whole screen and the top line with it
		ForgetScreen();
#endif
	}

	EndTime = Platform_Millis();

#if SCREENBUFFER == 0
	CHand_Move(Hand);

	//the hand, only when it really moved or came or went: where it was painted last and where
	//it stands now
	if ((Hand->X != paintedHandX) || (Hand->Y != paintedHandY) || (Hand->Hidden != paintedHandHidden))
	{
		if (!paintedHandHidden)
			MarkHandAt(paintedHandX, paintedHandY);
		if (!Hand->Hidden)
			MarkHandAt(Hand->X, Hand->Y);
	}

	//the carried piece, only when a button changed the board: the square it was painted in and
	//the one it is in now. A piece that is held still is not a change
	if (boardMoved)
	{
		if (paintedPieceAny)
			MarkDirty(paintedPieceX0, paintedPieceY0,
			          (int16_t)(paintedPieceX1 - paintedPieceX0), (int16_t)(paintedPieceY1 - paintedPieceY0));
		int16_t x0, y0, x1, y1;
		if (ActiveBlockBounds(&x0, &y0, &x1, &y1))
			MarkDirty(x0, y0, (int16_t)(x1 - x0), (int16_t)(y1 - y0));
	}

	if (dirtyAny)
	{
		const int16_t w = dirtyX1 - dirtyX0, h = dirtyY1 - dirtyY0;
		if (BandRender_Begin(ImgRoomBackground, dirtyX0, dirtyY0, w, h))
		{
			//the background, the blocks and the hand put together in memory and sent a strip at
			//a time, so the display never shows the background on its own. The drawing calls go
			//into the strip by themselves while one is open, see bandrender.h
			while (BandRender_Next())
			{
				DrawPlayFieldRect(BandRender_StripX(), BandRender_StripY(),
				                  BandRender_StripW(), BandRender_StripH());
				CHand_Draw(Hand);
			}
		}
		else
		{
			//no room for the strip buffer: the plain way, which does show the background for a
			//moment where something moved
			drawImageRLEPart(dirtyX0, dirtyY0, dirtyX0, dirtyY0, w, h, ImgRoomBackground,
			                 fullScreenWidth, fullScreenHeight, false);
			DrawPlayFieldRect(dirtyX0, dirtyY0, w, h);
			CHand_Draw(Hand);
		}
		dirtyAny = false;
		//what the screen now holds, which the next frame marks against
		paintedHandX = Hand->X;
		paintedHandY = Hand->Y;
		paintedHandHidden = Hand->Hidden;
		paintedPieceAny = ActiveBlockBounds(&paintedPieceX0, &paintedPieceY0, &paintedPieceX1, &paintedPieceY1);
	}

	//the top line last, so the board can never be painted over it
	{
		char text[PanelTextMax];
		PanelLevelText(text);
		PaintPanelText(PanelLevelX, text, shownLevelText);
		PanelTimeText(text);
		PaintPanelText(PanelTimeX, text, shownTimeText);
	}
#else
	drawImageRLE(0,0,fullScreenWidth,fullScreenHeight,ImgRoomBackground);
	DrawPanel();
	DrawPlayField();
	CHand_Move(Hand);
	CHand_Draw(Hand);
#endif

	if(GameState != GSGame)
		GameDeInit();
}
