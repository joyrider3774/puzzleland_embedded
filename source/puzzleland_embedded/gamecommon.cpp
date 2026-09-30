
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helperfuncs.h"
#include "commonvars.h"
#include "sound.h"
#include "savestate.h"
#include "gamecommon.h"



int GetLevel(char *Password)
{
	int Result,Teller;
	Result = -1;
	for (Teller = 0;Teller < 36;Teller++)
	{
		if (strcmp(Password,LevelPasswords[Teller]) ==0)
		{
			Result = Teller;
			break;
		}
	}
	return Result;
}

//the colour the top line is printed in. The text and its background are the same, which is how
//printText is told to draw the figures only and leave what is behind them alone
static inline uint16_t PanelColor(void)
{
	return SCREEN.color565(255,255,255);
}

void PanelLevelText(char* out)
{
	snprintf(out, PanelTextMax, "Lvl:%d", Level);
}

void PanelTimeText(char* out)
{
	//Platform_Millis() counts milliseconds, the time is shown in hours, minutes and seconds.
	//Milliseconds and not microseconds, as those wrap around after 71 minutes and a level can be
	//sat in for longer than that
	const uint32_t elapsed = (EndTime - StartTime) / 1000;
	snprintf(out, PanelTextMax, "Time: %02u:%02u:%02u",
	         (unsigned)(elapsed / 3600), (unsigned)((elapsed % 3600) / 60), (unsigned)(elapsed % 60));
}

void DrawPanel()
{
	char text[PanelTextMax];
	PanelLevelText(text);
	tftPrint(PanelLevelX, PanelY, text, strlen(text), PanelColor(), PanelColor(), 1);
	PanelTimeText(text);
	tftPrint(PanelTimeX, PanelY, text, strlen(text), PanelColor(), PanelColor(), 1);
}

//one character of the top line, at the cell the character with that index sits in
void DrawPanelChar(int16_t textX, int8_t index, char c)
{
	const char one[2] = { c, 0 };
	tftPrint((int16_t)(textX + index * PanelCharW), PanelY, one, 1, PanelColor(), PanelColor(), 1);
}

void DrawPlayField()
{
	//the whole board
	DrawPlayFieldRect(MinPlayAreaX, MinPlayAreaY, BlockWidth * Cols + 1, BlockHeight * Rows + 1);
}

//What one square of the board is painted with and which of its sides carry the black outline of
//a shape: a side carries it where the neighbour is a different block. The border squares (a
//negative value) carry none. bandrender.cpp asks the same thing when it puts a strip together,
//so both ways of painting agree on every pixel
const uint8_t* PlayFieldCellImage(int Layer, int X, int Y, uint8_t* edges)
{
	const int value = PlayField[Layer][X][Y];
	*edges = 0;
	if (value > 0)
	{
		if ((X == 0) || (PlayField[Layer][X - 1][Y] != value))
			*edges |= CellEdgeLeft;
		if ((X == Cols - 1) || (PlayField[Layer][X + 1][Y] != value))
			*edges |= CellEdgeRight;
		if ((Y == 0) || (PlayField[Layer][X][Y - 1] != value))
			*edges |= CellEdgeTop;
		if ((Y == Rows - 1) || (PlayField[Layer][X][Y + 1] != value))
			*edges |= CellEdgeBottom;
		//layer 1 is the piece being carried, which is painted lighter than the board
		return (Layer == 1) ? ImgBlockActiveImage : ImgBlockImage;
	}
	if (value < 0)
		return BorderImages[abs(value) - 1];
	return NULL;
}

//Only the blocks whose square touches the rectangle are drawn. A block paints BlockWidth by
//BlockHeight pixels and the outline of a shape runs one pixel past that on the right and the
//bottom, so a block counts as reaching into the rectangle when its square plus that pixel does.
//Painting a part of the screen instead of all of it is what keeps a device without a screen
//buffer from repainting what did not change, see the repaint in game.cpp
void DrawPlayFieldRect(int clipX, int clipY, int clipW, int clipH)
{
	const int clipX1 = clipX + clipW;
	const int clipY1 = clipY + clipH;
	const uint16_t outline = SCREEN.color565(0,0,0);
	int Layers,X,Y;
	for (Layers = 0;Layers <= 1;Layers++)
		for (X=Cols-1;X>=0;X--)
		{
			const int blockX = XOffsetGame + X * BlockWidth;
			if ((blockX + BlockWidth + 1 <= clipX) || (blockX >= clipX1))
				continue;
			for(Y=Rows-1;Y>=0;Y--)
			{
				const int blockY = YOffsetGame + Y * BlockHeight;
				if ((blockY + BlockHeight + 1 <= clipY) || (blockY >= clipY1))
					continue;

				uint8_t edges;
				const uint8_t* image = PlayFieldCellImage(Layers, X, Y, &edges);
				if (!image)
					continue;

				drawImage(blockX, blockY, BlockWidth, BlockHeight, image);
				if (edges & CellEdgeLeft)
					fillRect(blockX, blockY, 1, BlockHeight, outline);
				if (edges & CellEdgeRight)
					fillRect(blockX + BlockWidth, blockY, 1, BlockHeight + 1, outline);
				if (edges & CellEdgeTop)
					fillRect(blockX, blockY, BlockWidth, 1, outline);
				if (edges & CellEdgeBottom)
					fillRect(blockX, blockY + BlockHeight, BlockWidth, 1, outline);
			}
		}
}




void LoadSettings()
{
	initSaveState();
	setSoundOn(isSoundOnSaveState());
	setMusicOn(isMusicOnSaveState());
}

void SaveSettings()
{
	setOptionsSaveState(isSoundOn(), isMusicOn());
}
