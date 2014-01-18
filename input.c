#include "input.h"
#include <stdio.h>
#include <stdlib.h>

void InitializeKeyPresses(InputDeviceValues_t *keys)
{
  keys->bSpace = 0;
  keys->bEsc = 0;
  keys->bR = 0;
  keys->bZ = 0;

  keys->bLeftClick = 0;
  keys->scrollVal = 0;
  keys->mouseMotionX = 0;
  keys->mouseMotionY = 0;
}

char CheckInput(InputDeviceValues_t *keys)
{
  SDL_Event event;
  if (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      keys->bEsc = 1;
    }

    if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_ESCAPE)
        keys->bEsc = 1;

      if (event.key.keysym.sym == SDLK_SPACE)
        keys->bSpace = 1;

      if (event.key.keysym.sym == SDLK_r)
        keys->bR = 1;

      if (event.key.keysym.sym == SDLK_z)
        keys->bZ = 1;
    }

    /* UGGGH this is so hackish. */
    /*if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      printf("Pressed button: %d\n", event.button.button);
      if (event.button.button == SDL_BUTTON_LEFT)
        keys->bLeftClick = 1;
    }

    if (event.type == SDL_MOUSEBUTTONUP)
    {
      printf("Released button: %d\n", event.button.button);
      if (event.button.button == SDL_BUTTON_LEFT)
        keys->bLeftClick = 0;
    }*/
    if (SDL_GetMouseState(NULL,NULL)&SDL_BUTTON_LEFT)
      keys->bLeftClick = 1;
    else
      keys->bLeftClick = 0;

    if (event.type == SDL_MOUSEWHEEL)
    {
      keys->scrollVal = event.wheel.y;
      //sprintf("Scrollval: %d\n", event.wheel.y);
    }

    if (event.type == SDL_MOUSEMOTION)
    {
      keys->mouseMotionX = event.motion.xrel;
      keys->mouseMotionY = event.motion.yrel;
      //printf("x,y motion: %d,%d\n", event.motion.xrel, event.motion.yrel);
    }
  }

  return 1;
}

char HandleInput(ThreadedLifeContext_t *worldContext,
  LifeGameGraphicsContext_t *graphicsContext,
  InputDeviceValues_t * const keys)
{
  if (keys->bZ)
  {
    SDL_LockMutex(worldContext->lock);
    worldContext->bRandomize = 1;
    SDL_UnlockMutex(worldContext->lock);
  }

  if (keys->bR)
  {
    //reload the file
    SDL_LockMutex(worldContext->lock);
    worldContext->bReloadFile = 1;
    SDL_UnlockMutex(worldContext->lock);
  }

  if (keys->bSpace)
  {
    //pause the simulation or unpause it.
    if (worldContext->bSimulating)
      worldContext->bSimulating = 0;
    else
      worldContext->bSimulating = 1;
  }

  if (keys->bEsc)
  {
    //we have to unlock the threads by setting bSimulating to 1 otherwise
    //they will hang when we try to quit.
    worldContext->bSimulating = 1;
    worldContext->bRunning = 0;
  }

  if (keys->scrollVal != 0)
  {
    //zoom by scroll wheel
    Vector3_t scaleModifier; 
    Vector3Set(&scaleModifier, 
      (float)keys->scrollVal * 0.075, 
      (float)keys->scrollVal * 0.075, 
      0.0);
    Vector3_t currentScale;
    GetGameGraphicsScale(&currentScale, graphicsContext);
    Vector3_t newScale;
    Vector3Add(&newScale, &currentScale, &scaleModifier);
    SetGameGraphicsScale(graphicsContext, &newScale);
  }

  if (keys->bLeftClick)
  {
    //printf("Translating: %d,%d\n", keys->mouseMotionX, keys->mouseMotionY);
    Vector3_t translationModifier;
    Vector3Set(&translationModifier,
      (float)keys->mouseMotionX * 0.00075,
      (float)keys->mouseMotionY * -0.00075, //flipped Y axis, remember?
      0.0);
    Vector3_t currentTranslation;
    GetGameGraphicsTranslation(&currentTranslation, graphicsContext);
    Vector3_t newTranslation;
    Vector3Add(&newTranslation, &currentTranslation, &translationModifier);
    SetGameGraphicsTranslation(graphicsContext, &newTranslation);
  }

  return 1;
}