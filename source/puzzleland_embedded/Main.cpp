#include <stdio.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "game.h"
#include "stageclear.h"
#include "nextstage.h"
#include "options.h"
#include "stageselect.h"
#include "oldmanspeaking.h"
#include "passwordentry.h"
#include "titlescreen.h"
#include "intro.h"
#include "credits.h"
#include "gamecommon.h"
#include "chand.h"
#include "commonvars.h"
#include "helperfuncs.h"
#include "sound.h"

//The program itself, Game_Setup and Game_Loop are called by the device's own source

const uint32_t timePerFrame =  1000000 / FRAMERATE;
//The frame rate in hundredths, which is what the debug header prints. It is not a float:
//no device here has floating point in hardware, and the software that stands in for it
//costs kilobytes of flash for a figure nothing but that header ever reads
static uint32_t frameRate = 0;
static uint32_t currentTime = 0, lastTime = 0, frameTime = 0;
static bool endFrame = true;
bool webAppStore = false;
static bool debugMode = false;

static uint32_t getFreeRam() { 
  return Platform_FreeHeap();
}

static uint32_t getFreeStack() {
	return Platform_FreeStack();
}

//lowest free heap seen since boot, sampled at the end of Game_Setup and of every frame.
//Something allocated and freed again within one frame does not show up here
static uint32_t lowestFreeRam = UINT32_MAX;

static void trackLowestFreeRam()
{
    uint32_t freeRam = getFreeRam();
    if (freeRam < lowestFreeRam)
        lowestFreeRam = freeRam;
}

static void printDebugCpuRamLoad()
{
    if(debugMode || FORCEDEBUG)
    {
        //the text is only put together a few times a second: every frame it would cost the
        //formatting and the heap and stack readings for figures nobody can read that fast.
        //It is still drawn every frame, the board may have been drawn over it
        static char debuginfo[80] = "";
        static uint32_t lastUpdate = 0;
        uint32_t now = Platform_Micros();
        if ((debuginfo[0] == '\0') || (now - lastUpdate >= 250000))
        {
            lastUpdate = now;
            int fps_int = (int)(frameRate / 100);
            int fps_frac = (int)(frameRate % 100);
            //S is the least sketch stack that has been free since boot, out of 4096 bytes
            //L: is the lowest free heap since boot, in the same column as R: on the line above
            //The figures go over as signed, which every one of these devices prints. The CHGame links a
            //cut down snprintf that writes the signed conversions and quietly drops the unsigned ones,
            //so "R:%u" came out as "R:" and nothing after it. None of the three ever comes near what an
            //int holds: the largest possible is the whole of the device's RAM
            snprintf(debuginfo, sizeof(debuginfo), "F:%3d.%2d R:%3d \nS:%4d   L:%3d ", fps_int, fps_frac, (int)getFreeRam(), (int)getFreeStack(), (int)lowestFreeRam);
            //Platform_Log("%s\n", debuginfo);
        }
        tftPrint(0, 0, debuginfo, strlen(debuginfo), SCREEN.color565(255,255,255), SCREEN.color565(0,0,0), 1);
    }
}

void Game_Setup(void)
{   
    //webAppStore is set in Platform_Init
    Platform_Init("Puzzleland v1.0");
    if(!webAppStore)
    {
        Platform_Log("Free Ram at boot game: %6" PRIu32 "\n", getFreeRam());
          debugMode = false;        
		//without the opening picture there is no intro to start on, see INTROSCREEN
		GameState = INTROSCREEN ? GSIntroInit : GSTitleScreenInit;
		OldTime = 0;

		preloadImages();
		initSound();
		initMusic();
		LoadSettings();
		Hand = CHand_Create();
        //With a 1 bpp buffer, the colours its set and clear bits are shown in. A bit is set for a
        //pixel brighter than mid grey (see SetBufferBit), so set is white and clear is black
        //whichever skin is built in. The skin's own colours are what is drawn, not what it shows
        //as: the black & white skin draws its white on black, the colour skin its black on white,
        //and either way the brightness of what was drawn is what comes out
        Platform_SetBufferColors(SCREEN.color565(255,255,255), SCREEN.color565(0,0,0));
        trackLowestFreeRam();
        currentTime = Platform_Micros();
        lastTime = 0;
    }
    else
    {
        //webappstore stuff
    }
}

void Game_Loop(void)
{
    if(!webAppStore)
    {        
        currentTime = Platform_Micros();
        frameTime  = currentTime - lastTime;
    #if FPSLOCK
        if((frameTime < timePerFrame) || !endFrame)
           return;
    #else
        //no lock, a frame starts as soon as the last one is done
        if(!endFrame)
           return;
    #endif
        endFrame = false;
        //without the lock two frames can start within the same microsecond on a fast PC
        //a second in microseconds, times a hundred so the answer is in hundredths
        frameRate = 100000000UL / (frameTime ? frameTime : 1);
        lastTime = currentTime;    
        prevButtons = currButtons;
        currButtons = Platform_GetButtons();
        musicTimer();
        //debug mode is toggled with (A) and left and down together, whichever of them is
        //pressed last. The dpad alone would trigger it while playing
        const uint8_t debugCombo = BUTTON_A | BUTTON_LEFT | BUTTON_DOWN;
        if(((currButtons & debugCombo) == debugCombo) && ((prevButtons & debugCombo) != debugCombo))
            debugMode = !debugMode;

    	//gamestate handling   
        switch(GameState)
		{
#if INTROSCREEN
			case GSIntroInit :
			case GSIntro :
				Intro();
				break;
#endif
			case GSTitleScreenInit :
			case GSTitleScreen :
				TitleScreen();
				break;
			case GSOldManSpeakingInit :
			case GSOldManSpeaking :
				OldManSpeaking();
				break;
			case GSPasswordEntryInit :
			case GSPasswordEntry :
				PasswordEntry();
				break;
			case GSCreditsInit :
			case GSCredits :
				Credits();
				break;
			case GSOptionsInit :
			case GSOptions :
				Options();
				break;
			case GSGameInit :
			case GSGame :
				Game();
				break;
			case GSStageClearInit:
			case GSStageClear:
				StageClear();
				break;
			case GSNextStageInit:
			case GSNextStage:
				//The first room a build has arrives at a single building; every one after that walks
				//between the one just left and the next. A build holding a run of the rooms starts at
				//FIRSTLEVEL, not at 0, and walking in from a building it has not got would read past
				//its end of the table
				if (Level == FIRSTLEVEL)
					NextStageLevel0();
				else
					NextStageLevel1to35();
				break;
			case GSStageSelectInit:
			case GSStageSelect:
				StageSelect();
				break;
			default:
				break;
		}

        trackLowestFreeRam();
        printDebugCpuRamLoad();
        Platform_PresentFrame();
        endFrame = true;
    }
    else
    {
        //webappstore stuff
        static uint32_t prev = 0;
        if(Platform_Micros() - prev > 1000000)
        {
            prev = Platform_Micros();
            Platform_Log("Free Ram webappstore: %6" PRIu32 "\n", getFreeRam());
        }
    }
}
