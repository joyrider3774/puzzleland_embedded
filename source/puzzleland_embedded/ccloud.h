#ifndef CCLOUD_H
#define CCLOUD_H

#include <stdint.h>

typedef enum {Big,Small} CloudStyles;

typedef struct CCloud CCloud;
struct CCloud
{
 	float X,Xi;
 	uint8_t Y, Width, Height;
	const uint8_t *Image;
} ;

void CCloud_Destroy(CCloud* Cloud);
void CCloud_Move(CCloud* Cloud);
void CCloud_Draw(CCloud* Cloud);
CCloud* CCloud_Create(const uint8_t XIn,const uint8_t YIn,float XiIn,CloudStyles Style);

#endif