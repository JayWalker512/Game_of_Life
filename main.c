//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc main.c threadlife.c graphics.c lifegraphics.c -o gameoflife -pthread -Wall -std=c99 -I/usr/local/include/SDL2 -lSDL2 -I/usr/include/GL -lGL -lGLEW -lm -Wall -g
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "lifegraphics.h"
#include <SDL2/SDL.h>

int main(int argc, char **argv)
{
  SDL_Window *window = InitSDL(1024, 768, "Game of Life", 0);
  SDL_GLContext glContext = InitSDL_GL(window);

  const LifeWorldDim_t worldWidth = 400;
  const LifeWorldDim_t worldHeight = 300;
  ThreadedLifeContext_t *worldContext = CreateThreadedLifeContext(worldWidth, worldHeight,
    0, 1);
  if (worldContext == NULL)
  {
    printf("Failed to ThreadedLifeContext_t!\n");
    return 1;
  }

  LifeGraphicsContext_t *graphicsContext = 
    CreateLifeGraphicsContext(worldWidth, worldHeight, "vs1.glsl", "fs1.glsl");
  if (graphicsContext == NULL)
  {
    printf("Failed to create LifeGraphicsContext_t!\n");
    return 1;
  }

  //create and start the simulation thread.
  SDL_Thread *lifeThread = SDL_CreateThread((SDL_ThreadFunction)&ThreadLifeMain, 
    "SimThread", worldContext);
  if (lifeThread == NULL)
  {
    printf("Couldn't create thread!\n");
    return 1;
  }

  //we start this loop in main thread that only does rendering and input
  while (worldContext->bRunning)
  {
    //Draw world so we can see initial state. 
    SyncWorldToScreen(window, worldContext, graphicsContext, 60);

    char bRandomizeWorld = 0;
    worldContext->bRunning = CheckInput(&bRandomizeWorld);
    if (bRandomizeWorld)
    {
      SDL_LockMutex(worldContext->lock);
      worldContext->bRandomize = 1;
      SDL_UnlockMutex(worldContext->lock);
    }
  }

  //cleaning up
  SDL_WaitThread(lifeThread, NULL);
  DestroyThreadedLifeContext(worldContext);
  DestroyLifeGraphicsContext(graphicsContext);
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

char CheckInput(char *bRandomizeWorld)
{
  SDL_Event event;
  if (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      return 0; //return 0 if we want to quit
    }

    if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_ESCAPE)
        return 0;

      if (event.key.keysym.sym == SDLK_SPACE)
        *bRandomizeWorld = 1;
    }
  }

  return 1;
}
