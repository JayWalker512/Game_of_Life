//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc main.c threadlife.c graphics.c lifegraphics.c -o gameoflife -pthread -Wall -std=c99 -I/usr/local/include/SDL2 -lSDL2 -I/usr/include/GL -lGL -lGLEW -lm -Wall -g
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "lifegraphics.h"
#include <SDL2/SDL.h>

int main(int argc, char **argv)
{
  SDL_Window *window = InitSDL(960, 600, "Game of Life", 0);
  SDL_GLContext glContext = InitSDL_GL(window);

  //create the thread context, this is the threads argument
  const LifeWorldDim_t worldWidth = 272;
  const LifeWorldDim_t worldHeight = 152;
  ThreadWorldContext_t worldContext;
  worldContext.front = NewLifeWorld(worldWidth, worldHeight);
  worldContext.back = NewLifeWorld(worldWidth, worldHeight);

  //create graphics context
  LifeGraphicsContext_t graphicsContext;
  graphicsContext.pWorldRenderBuffer = NewLifeWorld(worldWidth, worldHeight);
  graphicsContext.pQuadDrawData = NewQuadDataBuffer(worldWidth * worldHeight);
  if (!SetQuadShader(graphicsContext.pQuadDrawData, BuildShaderProgram("vs1.glsl", "fs1.glsl")))
  {
    printf("Couldn't set shader!\n");
  }

  /*if (pthread_mutex_init(&worldContext.lock, NULL) != 0)
  {
    printf("Couldn't init mutex!");
    return 1;
  }*/
  worldContext.lock = SDL_CreateMutex();
  if (!worldContext.lock)
  {
    printf("Couldn't init mutex!");
    return 1;
  }

  //still single threaded, but this action requires locking with threads!
  RandomizeWorldStateBinary(&worldContext);
  worldContext.bRandomize = 0; //needs to be initialied to use in thread 2!
  worldContext.bRunning = 1;

  //here we spawn the thread that will run the simulation
  /*pthread_t lifeThread;
  int err = pthread_create(&lifeThread, NULL, &ThreadLifeMain, &worldContext);
  if (err != 0)
  {
    printf("Couldn't create thread: %s\n", strerror(err));
    return 1;
  }*/
  SDL_Thread *lifeThread = SDL_CreateThread((SDL_ThreadFunction)&ThreadLifeMain, 
    "SimThread", &worldContext);
  if (lifeThread == NULL)
  {
    printf("Couldn't create thread!\n");
    return 1;
  }

  //we start this loop in main thread that only does rendering and inpu
  while (worldContext.bRunning)
  {
    //Draw world so we can see initial state. 
    SyncWorldToScreen(window, &worldContext, &graphicsContext, 60);

    char bRandomizeWorld = 0;
    worldContext.bRunning = CheckInput(&bRandomizeWorld);
    if (bRandomizeWorld)
    {
      SDL_LockMutex(worldContext.lock);
      worldContext.bRandomize = 1;
      SDL_UnlockMutex(worldContext.lock);
    }
  }

  //cleaning up
  SDL_WaitThread(lifeThread, NULL);
  SDL_DestroyMutex(worldContext.lock);
  DestroyLifeWorld(worldContext.front);
  DestroyLifeWorld(worldContext.back);
  DestroyLifeWorld(graphicsContext.pWorldRenderBuffer);
  DestroyQuadDrawData(graphicsContext.pQuadDrawData);
  SDL_GL_DeleteContext(glContext);
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
