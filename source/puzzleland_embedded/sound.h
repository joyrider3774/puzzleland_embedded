#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

#define musWinner 1

#define SFX_SUSTAIN 100

void playMenuSound(void);
void playDropBlockSound(void);
void playPickupBlockSound(void);
void playFlipBlockSound(void);
void playMenuSelectSound(void);
void playErrorSound(void);
void playBridgeSound(void);
void playStageEndSound(void);
void playElfSound(void);
void playRotateBlockSound(void);
void playTextSound(void);
void stopTextSound(void);

void initSound();
void SelectMusic(uint8_t musicFile, uint8_t loop);
void initMusic();
void setMusicOn(uint8_t value);
void setSoundOn(uint8_t value);
void musicTimer();
void processSound();
uint8_t isMusicOn();
uint8_t isSoundOn();

#endif