#include <stdint.h>
#include <string.h>
#include "helperfuncs.h"
#include "sound.h"
#include "cfairy.h"
#include "commonvars.h"
#include "gamecommon.h"
#include "oldmanspeaking.h"

//The speech box of the old man image is 119 pixels wide and 67 high inside its frame, and the
//font is six wide and nine high, so a line is at most 19 characters and a page at most 7 lines.
//A $ starts a new page, the player turns to it with A
const char* const Tekst[5] = {"Welcome to the\nworld of puzzle\nland! In level one\nwe'll start with\nsimple three-block\ncombinations ...$When you've\nmastered level one,\nwe'll add another\nblock.\nHang in there!",
								  "Congratulations you\nmade it! Now we'll\nboggle your mind,\nthere are 2339\ndiffrent\nconfigurations in\nthis stage!$Don't give up!\nRelax!",
								  "Hah! This next\nstage will surprise\nyou! You now have\n12 diffrent shapes\nto work with ...$Show me how far\nyou can go dude!",
								  "Well, well, well.\nYou seem to have\nmade it all this\nway! As a token of\nour appreciation\nfor your skill,...$Here's another\nsquare block! Take\nthe square block,\nand try it for the\nlast level!$It looks simple,\nbut there are 16146\nconfigurations!\n\nHey dude, you're on\nit again.",
          						  "Congratulations you\nmade it! You've\nearned the password\nfor the level\nselect!\nthe password is\n'davy'"};
int Lines,Chars,TextDelay,PageNr,TekstNr;
size_t NrOfChars;
size_t Teller;
//the pages a text is split into by its '$' marks, and how long one page gets. The longest
//text here is 3 pages of 7 lines, 110 characters, these leave room for a longer one and the
//loop below keeps within them. 100 by 255 was 25 KB of RAM, more than a Game Boy Advance has
#define MaxTextPages 4
#define MaxTextChars 160
char List[MaxTextPages][MaxTextChars];


    
void OldManSpeakingInit()
{
	Fairy = CFairy_Create(100,100,6);
	switch (Level)
	{
		case 0:
		case 11:
		case 12:
		case 35:
		case 36:
			TextDelay = 0;
			NrOfChars = 0;
			Lines = 0;
			Chars =0;
			PageNr = 0;
			switch (Level)
			{
				case 0:
					TekstNr = 0;
					break;
				case 11:
					TekstNr = 1;
					break;
				case 12:
					TekstNr = 2;
					break;
				case 35:
					TekstNr = 3;
					break;
				case 36:
					TekstNr = 4;
					break;
				default: TekstNr = 0;
			}
			for(Teller=0;Teller<strlen(Tekst[TekstNr]);Teller++)
			{
				if (Tekst[TekstNr][Teller] == '$')
				{
					List[Lines][Chars] = '\0';
					//a text with more pages than there is room for stops here
					if (Lines + 1 >= MaxTextPages)
						break;
					Lines++;
					Chars=0;
				}
				else
				{
					//the rest of a page longer than there is room for is left out
					if (Chars < MaxTextChars - 1)
					{
						List[Lines][Chars] = Tekst[TekstNr][Teller];
						Chars++;
					}
				}
			}
			List[Lines][Chars] = '\0';
			playTextSound();
		break;
		default:
			GameState = GSNextStageInit;
			break;
	}
}

void OldManSpeakingDeInit()
{
	stopTextSound();
	CFairy_Destroy(Fairy);
}


void OldManSpeaking()
{
	if(GameState == GSOldManSpeakingInit)
	{
		OldManSpeakingInit();
		if(GameState == GSOldManSpeakingInit)
			GameState -= GSInitDiff;
		needRedraw = 1;
	}

	if (GameState == GSOldManSpeaking)
	{
		if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
		{
			GameState = GSTitleScreenInit;
		}
		
		if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
		{
			if (PageNr < Lines)
			{
				PageNr++;
				NrOfChars = 0;
				playTextSound();
				//a new page, the box is emptied and filled again
				needRedraw = 1;
			}
			else
				if (Level < 36)
					GameState = GSNextStageInit;
				else
					GameState = GSCreditsInit;
		}
			/*			case SDLK_ESCAPE:
							Mix_HaltMusic();
							GameState = GSTitleScreenInit;
							break;
			*/
		
		if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
		{
			if (Level < 36)
				GameState = GSNextStageInit;
			else
				GameState = GSCreditsInit;
		}
#if SCREENBUFFER == 0
		//Without a screen buffer every drawing call goes straight to the display, so anything
		//that moves means painting the whole screen again: the old man, the box and the text
		//for one beat of a wing or one character more. The page is put on the screen in one go
		//and the fairy stands still, which leaves this screen painted exactly once per page
		if (NrOfChars < strlen(List[PageNr]))
		{
			NrOfChars = strlen(List[PageNr]);
			needRedraw = 1;
		}
		else
			stopTextSound();
#else
		//the fairy beats her wings and the text types itself out, both of them only now and
		//then: a frame where neither of them changed is left on the screen as it is
		if (CFairy_Move(Fairy))
			needRedraw = 1;

		TextDelay++;
		if (TextDelay == 2)
		{
			if (NrOfChars < strlen(List[PageNr]))
			{
				NrOfChars++;
				needRedraw = 1;
			}
			else
				stopTextSound();
			TextDelay = 0;
		}
#endif

		if (needRedraw)
		{
			drawImageRLE(0,0,fullScreenWidth, fullScreenHeight, ImgOldMan);
			CFairy_Draw(Fairy);
			tftPrint(7, 24,List[PageNr], NrOfChars, ColorText,ColorText,1);
			needRedraw = 0;
		}

	}
	
	if(GameState != GSOldManSpeaking)
		OldManSpeakingDeInit();
}
