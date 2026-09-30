#include <inttypes.h>
#include "helperfuncs.h"
#include "sound.h"
#include "commonvars.h"
#include "gamecommon.h"
#include "stageclear.h"


bool KaderVisible;

void StageClearInit()
{
	KaderVisible = true;
}

void StageClearDeInit()
{

}

void StageClear()
{
	if(GameState == GSStageClearInit)
	{
		StageClearInit();
		GameState -= GSInitDiff;
		needRedraw = 1;
	}

	if ((currButtons & BUTTON_R) && !(prevButtons & BUTTON_R))
	{
		GameState = GSTitleScreenInit;
	}

	if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
		GameState = GSOldManSpeakingInit;
	if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
	{
		//B shows and hides the frame over the solved room
		KaderVisible = !KaderVisible;
		needRedraw = 1;
	}

	//the board stands still here, only the frame comes and goes
	if (needRedraw)
	{
		drawImageRLE(0,0,fullScreenWidth, fullScreenHeight, ImgRoomBackground);
		DrawPanel();
		DrawPlayField();
		if (KaderVisible)
		{
			drawImageRLETransparent((WINDOW_WIDTH - stageClearKaderWidth)>>1, YOffsetGame + 20, stageClearKaderWidth, stageClearKaderHeight, ImgStageClearKader);
		}
		needRedraw = 0;
	}

	if(GameState != GSStageClear)
		StageClearDeInit();
}
