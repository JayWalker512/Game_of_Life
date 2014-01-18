#ifndef INPUT_H
#define INPUT_H

#include "threadlife.h"
#include "lifegraphics.h"

//this could certainly be more expansive/elegant, but you know what they say
//about future-coding...
typedef struct InputDeviceValues_s {
	//keys
  int esc[2];
  int space[2];
  int r[2];
  int z[2];
  int c[2];
  int plus[2];
  int minus[2];

  //mouse
  int leftClick[2];
  int rightClick[2];
  int scrollVal[2];
  int mouseMotionX[2];
  int mouseMotionY[2];
} InputDeviceValues_t;

void InitializeKeyPresses(InputDeviceValues_t *keys);
char CheckInput(InputDeviceValues_t *keys);
char HandleInput(ThreadedLifeContext_t *worldContext, 
  LifeGameGraphicsContext_t *graphicsContext, 
  InputDeviceValues_t * const keys);

#endif