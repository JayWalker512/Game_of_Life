//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc main.c threadlife.c graphics.c lifegraphics.c loadfile.c -o gameoflife -pthread -Wall -pedantic -std=c99 -I/usr/local/include/SDL2 -lSDL2 -I/usr/include/GL -lGL -lGLEW -lm -Wall -g
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "lifegraphics.h"
#include "loadfile.h"
#include <SDL2/SDL.h>

typedef struct LifeArgOptions_s {
  LifeWorldDim_t worldWidth;
  LifeWorldDim_t worldHeight;
  int resX;
  int resY;
  char bFullScreen;
  char bBenchmarking;
} LifeArgOptions_t;

static char CheckInput(char *bRandomizeWorld);
static char ParseArgs(LifeArgOptions_t *options, int argc, char **argv);

int main(int argc, char **argv)
{
  LifeArgOptions_t options;
  ParseArgs(&options, argc, argv);

  SDL_Window *window = InitSDL(options.resX, options.resY, 
    "Game of Life", options.bFullScreen);
  SDL_GLContext glContext = InitSDL_GL(window);

  ThreadedLifeContext_t *worldContext = 
    CreateThreadedLifeContext(options.worldWidth, options.worldHeight,
    0, 1);
  if (worldContext == NULL)
  {
    printf("Failed to ThreadedLifeContext_t!\n");
    return 1;
  }

  if (LoadLifeWorld(worldContext->front, "data/collision.life", 1) == 0)
  {
    printf("Failed to load file!\n");
  }

  LifeGraphicsContext_t *graphicsContext = 
    CreateLifeGraphicsContext(options.worldWidth, options.worldHeight,
    "shaders/vs1.glsl", "shaders/fs1.glsl");
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

char ParseArgs(LifeArgOptions_t *options, int argc, char **argv)
{
  //set default options first, then parse for changes.
  options->bFullScreen = 0;
  options->bBenchmarking = 0;
  options->worldWidth = 400;
  options->worldHeight = 300;
  options->resX = 1024;
  options->resY = 768;
  char *cvalue = NULL;

  int c;
  while ((c = getopt(argc, argv, "x:y:w:h:fb")) != -1 )
  {
    switch (c)
    {
      case 'f':
        options->bFullScreen = 1;
        break;
      case 'b':
        options->bBenchmarking = 1;
        break;
      case 'x':
        cvalue = optarg;
        options->resX = atoi(cvalue);
        break;
      case 'y':
        cvalue = optarg;
        options->resY = atoi(cvalue);
        break;
      case 'w':
        cvalue = optarg;
        options->worldWidth = atoi(cvalue);
        break;
      case 'h':
        cvalue = optarg;
        options->worldHeight = atoi(cvalue);
        break;
      case '?':
        if (optopt == 'x' || optopt == 'y' || optopt == 'w' || optopt == 'h')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
            "Unknown option character `\\x%x'.\n",
            optopt);
      default:
        puts("What's going on in here?!?!");
        break;
    }
  }
  return 1;
}
