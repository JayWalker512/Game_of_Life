#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

static int PushValueOnTwoElementArray(int *array, const int element)
{
	/* This function essentially treats a two-element array like a small
	stack. value in [0] gets moved to [1] and new element is inserted at
	[0]. Returns the element pushed on, or zero for failure. */

	if (array == NULL)
		return 0;

	*(array+1) = *(array+0);
	*(array+0) = element;
	return element;
}

void InitializeKeyPresses(InputDeviceValues_t *keys)
{

  keys->space[0] = keys->space[1] = 0;
  keys->esc[0] = keys->esc[1] = 0;
  keys->r[0] = keys->r[1] = 0;
  keys->z[0] = keys->z[1] = 0;
  keys->c[0] = keys->c[1] = 0;
  keys->plus[0] = keys->plus[1] = 0;
  keys->minus[0] = keys->minus[1] = 0;

  keys->leftClick[0] = keys->leftClick[1] = 0;
  keys->rightClick[0] = keys->rightClick[1] = 0;
  keys->scrollVal[0] = keys->scrollVal[1] = 0;
  keys->mouseMotionX[0] = keys->mouseMotionX[1] = 0;
  keys->mouseMotionY[0] = keys->mouseMotionY[1] = 0;
}

char CheckInput(InputDeviceValues_t *keys)
{
	//oh my god this function is so long and terribly verbose. 200 LoC function not cool!
	int escChanged = 0;
	int spaceChanged = 0;
	int rChanged = 0;
	int zChanged = 0;
	int cChanged = 0;
	int plusChanged = 0;
	int minusChanged = 0;

	int leftClickChanged = 0;
	int rightClickChanged = 0;
	int scrollValChanged = 0;
	int mouseMotionChanged = 0;

  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
  	if (event.key.repeat == 0)
    {
	    if (event.type == SDL_QUIT)
	    {
	      PushValueOnTwoElementArray(keys->esc, 1);
	      escChanged = 1;
	    }

	    if (event.type == SDL_KEYDOWN)
	    {
	      if (event.key.keysym.sym == SDLK_ESCAPE)
	      {
	      	PushValueOnTwoElementArray(keys->esc, 1);
	      	escChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_SPACE)
	      {
	      	PushValueOnTwoElementArray(keys->space, 1);
	      	spaceChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_r)
	      {
	      	PushValueOnTwoElementArray(keys->r, 1);
	      	rChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_z)
	      {
	      	PushValueOnTwoElementArray(keys->z, 1);
	      	zChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_c)
	      {
	      	PushValueOnTwoElementArray(keys->c, 1);
	      	cChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_PLUS)
	      {
	      	PushValueOnTwoElementArray(keys->plus, 1);
	      	plusChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_MINUS)
	      {
	      	PushValueOnTwoElementArray(keys->minus, 1);
	      	minusChanged = 1;
	      }
	    }

	    if (event.type == SDL_KEYUP)
	    {
	      if (event.key.keysym.sym == SDLK_ESCAPE)
	      {
	      	PushValueOnTwoElementArray(keys->esc, 0);
	      	escChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_SPACE)
	      {
	      	PushValueOnTwoElementArray(keys->space, 0);
	      	spaceChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_r)
	      {
	      	PushValueOnTwoElementArray(keys->r, 0);
	      	rChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_z)
	      {
	      	PushValueOnTwoElementArray(keys->z, 0);
	      	zChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_c)
	      {
	      	PushValueOnTwoElementArray(keys->c, 0);
	      	cChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_PLUS)
	      {
	      	PushValueOnTwoElementArray(keys->plus, 0);
	      	plusChanged = 1;
	      }

	      if (event.key.keysym.sym == SDLK_MINUS)
	      {
	      	PushValueOnTwoElementArray(keys->minus, 0);
	      	minusChanged = 1;
	      }
	    }

	    if (event.type == SDL_MOUSEBUTTONDOWN)
	    {
	      //printf("Pressed button: %d\n", event.button.button);
	      if (event.button.button == SDL_BUTTON_LEFT)
	      {
	      	PushValueOnTwoElementArray(keys->leftClick, 1);
	      	leftClickChanged = 1;
	      }

	      if (event.button.button == SDL_BUTTON_RIGHT)
	      {
	      	PushValueOnTwoElementArray(keys->rightClick, 1);
	      	rightClickChanged = 1;
	      }
	    }

	    if (event.type == SDL_MOUSEBUTTONUP)
	    {
	      //printf("Released button: %d\n", event.button.button);
	      if (event.button.button == SDL_BUTTON_LEFT)
	      {
	        PushValueOnTwoElementArray(keys->leftClick, 0);
	      	leftClickChanged = 1;
	      }

	      if (event.button.button == SDL_BUTTON_RIGHT)
	      {
	        PushValueOnTwoElementArray(keys->rightClick, 0);
	      	rightClickChanged = 1;
	      }
	    }

	    if (event.type == SDL_MOUSEWHEEL)
	    {
	      PushValueOnTwoElementArray(keys->scrollVal, event.wheel.y);
	      scrollValChanged = 1;
	    }

	    if (event.type == SDL_MOUSEMOTION)
	    {
	      PushValueOnTwoElementArray(keys->mouseMotionX, event.motion.xrel);
	      PushValueOnTwoElementArray(keys->mouseMotionY, event.motion.yrel);
	      mouseMotionChanged = 1;
	      //printf("x,y motion: %d,%d\n", event.motion.xrel, event.motion.yrel);
	    }
	  }
	}

  if (!escChanged)
  	PushValueOnTwoElementArray(keys->esc, keys->esc[0]);

  if (!spaceChanged)
  	PushValueOnTwoElementArray(keys->space, keys->space[0]);

  if (!rChanged)
  	PushValueOnTwoElementArray(keys->r, keys->r[0]);

  if (!zChanged)
  	PushValueOnTwoElementArray(keys->z, keys->z[0]);

  if (!cChanged)
  	PushValueOnTwoElementArray(keys->c, keys->c[0]);

  if (!plusChanged)
  	PushValueOnTwoElementArray(keys->plus, keys->plus[0]);

  if (!minusChanged)
  	PushValueOnTwoElementArray(keys->minus, keys->minus[0]);

  if (!leftClickChanged)
  	PushValueOnTwoElementArray(keys->leftClick, keys->leftClick[0]);

  if (!rightClickChanged)
  	PushValueOnTwoElementArray(keys->rightClick, keys->rightClick[0]);

  if (!scrollValChanged)
  	PushValueOnTwoElementArray(keys->scrollVal, 0);

  if (!mouseMotionChanged)
  {
  	PushValueOnTwoElementArray(keys->mouseMotionX, 0);
  	PushValueOnTwoElementArray(keys->mouseMotionY, 0);
  }

  return 1;
}

char HandleInput(ThreadedLifeContext_t *worldContext,
  LifeGameGraphicsContext_t *graphicsContext,
  InputDeviceValues_t * const keys)
{
  if (keys->z[0] && !keys->z[1]) //key pressed down
  {
    SDL_LockMutex(worldContext->lock);
    worldContext->bRandomize = 1;
    SDL_UnlockMutex(worldContext->lock);
  }

  if (keys->r[0] && !keys->r[1])
  {
    //reload the file
    SDL_LockMutex(worldContext->lock);
    worldContext->bReloadFile = 1;
    SDL_UnlockMutex(worldContext->lock);
  }

  if (keys->c[0] && !keys->c[1])
  {
    //clear the world.
    SDL_LockMutex(worldContext->lock);
    worldContext->bClearWorld = 1;
    SDL_UnlockMutex(worldContext->lock);
  }

  if (keys->space[0] && !keys->space[1])
  {
    //pause the simulation or unpause it.
    if (worldContext->bSimulating)
      worldContext->bSimulating = 0;
    else
      worldContext->bSimulating = 1;
  }

  if (keys->esc[0] && !keys->esc[1])
  {
    //we have to unlock the threads by setting bSimulating to 1 otherwise
    //they will hang when we try to quit.
    worldContext->bSimulating = 1;
    worldContext->bRunning = 0;
  }

  if (keys->scrollVal[0] != 0)
  {
    //zoom by scroll wheel
    Vector3_t scaleModifier; 
    Vector3Set(&scaleModifier, 
      (float)keys->scrollVal[0] * 0.075, 
      (float)keys->scrollVal[0] * 0.075, 
      0.0);
    Vector3_t currentScale;
    GetGameGraphicsScale(&currentScale, graphicsContext);
    Vector3_t newScale;
    Vector3Add(&newScale, &currentScale, &scaleModifier);
    SetGameGraphicsScale(graphicsContext, &newScale);
  }

  if (keys->leftClick[0]) //key down at all
  {
    //printf("Translating: %d,%d\n", keys->mouseMotionX, keys->mouseMotionY);
    Vector3_t translationModifier;
    Vector3Set(&translationModifier,
      (float)keys->mouseMotionX[0] * 0.00075,
      (float)keys->mouseMotionY[0] * -0.00075, //flipped Y axis, remember?
      0.0);
    Vector3_t currentTranslation;
    GetGameGraphicsTranslation(&currentTranslation, graphicsContext);
    Vector3_t newTranslation;
    Vector3Add(&newTranslation, &currentTranslation, &translationModifier);
    SetGameGraphicsTranslation(graphicsContext, &newTranslation);
  }

  return 1;
}