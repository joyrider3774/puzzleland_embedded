#ifndef CCLOUD_H
#define CCLOUD_H

#include <stdint.h>

typedef enum {Big,Small} CloudStyles;

//A cloud's place and speed are kept in 1/256 of a pixel rather than as floats: none of the
//devices has floating point in hardware, and the software that stands in for it costs about 2 KB
//of flash, which is more than the CHGame can spare. CLOUD_SPEED turns pixels a frame into that unit
//when the program is compiled, so CCloud_Create(120, 1, CLOUD_SPEED(-0.40f), Big) costs nothing
#define CLOUD_SPEED(pixels) ((int16_t)((pixels) * 256.0f))

typedef struct CCloud CCloud;
struct CCloud
{
 	int16_t X,Xi;
 	uint8_t Y, Width, Height;
	const uint8_t *Image;
} ;

//The cloud's left edge in pixels, which is what drawing and the dirty rectangles need: X is in
//1/256 of a pixel. Divided rather than shifted, so a cloud part way off the left edge rounds
//towards 0 as the float's (int) did
static inline int16_t CCloud_ScreenX(const CCloud* Cloud) { return (int16_t)(Cloud->X / 256); }

void CCloud_Destroy(CCloud* Cloud);
void CCloud_Move(CCloud* Cloud);
void CCloud_Draw(CCloud* Cloud);
CCloud* CCloud_Create(const uint8_t XIn,const uint8_t YIn,int16_t XiIn,CloudStyles Style);

#endif