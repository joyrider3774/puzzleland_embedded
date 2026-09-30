#ifndef GAMECOMMON_H
#define GAMECOMMON_H

#include <stdint.h>

//the sides of a block that carry the black outline of a shape, see PlayFieldCellImage
#define CellEdgeLeft   1
#define CellEdgeRight  2
#define CellEdgeTop    4
#define CellEdgeBottom 8

//What one square of the board is painted with: the image, or NULL when the square is empty, and
//in edges the sides of it that carry the outline. Both the plain painting and the strip painting
//of bandrender.cpp ask this, so there is one answer and they cannot drift apart
const uint8_t* PlayFieldCellImage(int Layer, int X, int Y, uint8_t* edges);

//The top line of the game screen: the level on the left and the clock beside it, both in the
//6 by 8 font. The places are here because game.cpp paints the line a character at a time, so
//only the figures that really moved on are drawn again
#define PanelY 4
#define PanelH 8
#define PanelCharW 6
#define PanelLevelX 2
#define PanelTimeX (2 + 7 * 6)
#define PanelTextMax 24

//the two texts as DrawPanel prints them, into a buffer of at least PanelTextMax characters
void PanelLevelText(char* out);
void PanelTimeText(char* out);

void DrawPlayField();
//only the blocks whose square touches the rectangle, see DrawPlayFieldRect in gamecommon.cpp
void DrawPlayFieldRect(int clipX, int clipY, int clipW, int clipH);
void DrawPanel();
//one character of the top line, textX being PanelLevelX or PanelTimeX
void DrawPanelChar(int16_t textX, int8_t index, char c);
int GetLevel(char *Password);
void SaveSettings();
void LoadSettings();
#endif