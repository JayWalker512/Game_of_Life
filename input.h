#ifndef INPUT_H
#define INPUT_H

#include "threadlife.h"
#include "lifegraphics.h"

typedef struct InputDeviceValues_s {
  char bEsc;
  char bSpace;
  char bR;
  char bZ;

  //mouse
  char bLeftClick;
  int scrollVal;
  int mouseMotionX;
  int mouseMotionY;
} InputDeviceValues_t;

void InitializeKeyPresses(InputDeviceValues_t *keys);
char CheckInput(InputDeviceValues_t *keys);
char HandleInput(ThreadedLifeContext_t *worldContext, 
  LifeGameGraphicsContext_t *graphicsContext, 
  InputDeviceValues_t * const keys);

#endif