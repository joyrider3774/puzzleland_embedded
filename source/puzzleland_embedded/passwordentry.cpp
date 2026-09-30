#include <string.h>
#include <inttypes.h>
#include "sound.h"
#include "helperfuncs.h"
#include "cpasswordselector.h"
#include "commonvars.h"
#include "gamecommon.h"
#include "bandrender.h"
#include "passwordentry.h"


int CharNr;
char Password[5];
CPasswordSelector* PasswordSelector;

void PasswordEntryInit()
{
	PasswordEntryCoolDown = 0;
	CharNr = 0;
	Password[0] = ' ';
	Password[1] = ' ';
	Password[2] = ' ';
	Password[3] = ' ';
	Password[4] = '\0';
	PasswordSelector = CPasswordSelector_Create();
}

void PassWordEntryDeInit()
{
	CPasswordSelector_Destroy(PasswordSelector);
}

//where a letter of the grid is printed
static int16_t LetterX(int X) { return (int16_t)(XOffsetPassword + X * PasswordCellWidth); }
static int16_t LetterY(int Y) { return (int16_t)(YOffsetPassword + Y * PasswordCellHeight); }

//The square one letter takes: the arrow, which stands a little in front of the letter, and the
//letter itself. The letters never change, only the arrow moves over them, so a move paints the
//square it left and the square it moved to and leaves the other twenty four alone. Painting the
//whole screen means the paper and all of the letters, and every letter is a write of its own,
//which on a device without a screen buffer is a stall every time the cursor moves
static void PaintLetter(int X, int Y, bool withArrow)
{
	const int16_t lx = LetterX(X), ly = LetterY(Y);
	//Only the gap in front of the letter, which is the whole of where the arrow stands. A letter
	//is 6 pixels and the cells are PasswordCellWidth apart, so the gap is PasswordCellWidth - 6.
	//The letters never change, so none of them is touched and none has to be printed again;
	//reaching as far left as the arrow's image does would wipe the letter of the column before
	const int16_t x = (int16_t)(lx - (PasswordCellWidth - 6));
	const int16_t y = (int16_t)(ly - 3);
	const int16_t w = (int16_t)(PasswordCellWidth - 6);
	const int16_t h = selectHeight;

#if PAPERBACKGROUND
	//only the piece of the paper behind this one square
	if (BandRender_Begin(ImgPaper, x, y, w, h))
	{
		while (BandRender_Next())
			if (withArrow)
				CPasswordSelector_Draw(PasswordSelector);
	}
	else
	{
		drawImageRLEPart(x, y, x, y, w, h, ImgPaper, fullScreenWidth, fullScreenHeight, false);
		if (withArrow)
			CPasswordSelector_Draw(PasswordSelector);
	}
#else
	//no paper to put back, so the ground it stood on is filled instead
	fillRect(x, y, w, h, ColorPaper);
	if (withArrow)
		CPasswordSelector_Draw(PasswordSelector);
#endif
}

//the word at the top, which only changes when a letter is picked or the word is thrown away
static void PaintPassword(void)
{
	const int16_t w = (int16_t)(strlen(Password) * 6);
	const int16_t x = (int16_t)((WINDOW_WIDTH >> 1) - (w >> 1));
#if PAPERBACKGROUND
	if (BandRender_Begin(ImgPaper, x, 8, w, 8))
	{
		while (BandRender_Next())
			;
	}
	else
		drawImageRLEPart(x, 8, x, 8, w, 8, ImgPaper, fullScreenWidth, fullScreenHeight, false);
#else
	fillRect(x, 8, w, 8, ColorPaper);
#endif
	tftPrint(x, 8, Password, strlen(Password), ColorText, ColorText, 1);
}

void PasswordEntry()
{
	//the word at the top changed, which is the only thing beside the arrow that ever moves here
	bool passwordChanged = false;
	
	if(GameState == GSPasswordEntryInit)
	{
		PasswordEntryInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}
	
	if (Password[3] != ' ')
	{
		if (PasswordEntryCoolDown > 0)
			PasswordEntryCoolDown--;
		if (PasswordEntryCoolDown == 0)
		{
			const int room = GetLevel(Password);
			//A word that names no room gives -1. So does one naming a room this build has not
			//got, as far as the player is concerned: the rooms are spread over several binaries,
			//see FIRSTLEVEL in defines.h, and a word for a room in another of them is refused
			//here rather than opening anything
			if ((room < 0) || ((room > 0) && !LEVELBUILT(room)))
			{
				Password[0] = ' ';
				Password[1] = ' ';
				Password[2] = ' ';
				Password[3] = ' ';
				playErrorSound();
				//the word at the top is wiped
				passwordChanged = true;
			}
			else if (room == 0)
			{
				//the first room's word, which opens the room select rather than a room
				GameState = GSStageSelectInit;
			}
			else
			{
				Level = room;
				GameState = GSOldManSpeakingInit;
			}
		}
	}

	if(GameState == GSPasswordEntry)
	{
		if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
		{
			GameState = GSTitleScreenInit;
		}
		
		if (PasswordEntryCoolDown == 0)
		{
			if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
			{
				GameState = GSTitleScreenInit;
			}

			if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
			{
				Password[CharNr] = Letters[CPasswordSelector_GetY(PasswordSelector)][CPasswordSelector_GetX(PasswordSelector)];
				playMenuSelectSound();
				CharNr++;
				if (CharNr > 3)
				{
					PasswordEntryCoolDown = (int)(FRAMERATE / 2);
					CharNr = 0;
				}
				//a letter more of the word at the top
				passwordChanged = true;
			}

			//where the arrow stood before the buttons were handled
			const uint8_t wasX = CPasswordSelector_GetX(PasswordSelector);
			const uint8_t wasY = CPasswordSelector_GetY(PasswordSelector);

			if ((currButtons & BUTTON_UP) && !(prevButtons & BUTTON_UP))
				CPasswordSelector_MoveUp(PasswordSelector);

			if ((currButtons & BUTTON_DOWN) && !(prevButtons & BUTTON_DOWN))
				CPasswordSelector_MoveDown(PasswordSelector);

			if ((currButtons & BUTTON_LEFT) && !(prevButtons & BUTTON_LEFT))
				CPasswordSelector_MoveLeft(PasswordSelector);

			if ((currButtons & BUTTON_RIGHT) && !(prevButtons & BUTTON_RIGHT))
				CPasswordSelector_MoveRight(PasswordSelector);

			//only a selection that really moved changes anything: the square it left and the
			//square it moved to, not the whole screen
			if (!needRedraw &&
			    ((CPasswordSelector_GetX(PasswordSelector) != wasX) ||
			     (CPasswordSelector_GetY(PasswordSelector) != wasY)))
			{
				PaintLetter(wasX, wasY, false);
				PaintLetter(CPasswordSelector_GetX(PasswordSelector),
				            CPasswordSelector_GetY(PasswordSelector), true);
			}
		}

		if (needRedraw)
		{
				#if PAPERBACKGROUND
			drawImageRLE(0, 0, fullScreenWidth, fullScreenHeight, ImgPaper);
			#else
			fillRect(0, 0, fullScreenWidth, fullScreenHeight, ColorPaper);
			#endif
			tftPrint((WINDOW_WIDTH >> 1) - ((strlen(Password)*6) >> 1), 8, Password, strlen(Password), ColorText,ColorText, 1);
			int LetterX,LetterY;
			for (LetterY = 0 ;LetterY < LetterRows;LetterY++)
				for (LetterX = 0;LetterX < LetterCols;LetterX++)
				{
					if(Letters[LetterY][LetterX] != '0')
					{
						char Letter[2] = "\0";
						Letter[0] = Letters[LetterY][LetterX];
						tftPrint(XOffsetPassword + LetterX * PasswordCellWidth, YOffsetPassword + LetterY * PasswordCellHeight, Letter, 1, ColorText, ColorText, 1);
					}
				}
			CPasswordSelector_Draw(PasswordSelector);
			needRedraw = 0;
		}
		else if (passwordChanged)
			PaintPassword();
	}
	
	if(GameState != GSPasswordEntry)
		PassWordEntryDeInit();
}
