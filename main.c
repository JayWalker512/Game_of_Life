//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc main.c threadlife.c graphics.c lifegraphics.c loadfile.c dirtyregion.c stack.c -o gameoflife -std=c99 -Wall -Wextra -pedantic -g -D_GNU_SOURCE -I/usr/local/include/SDL2 -lSDL2 -I/usr/include/GL -lGL -lGLEW -lm
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
  int regionSize;
  char bFullScreen;
  char bBenchmarking;
  char lifeFile[MAX_FILENAME_LENGTH];
} LifeArgOptions_t;

typedef struct KeyPresses_s {
  char bEsc;
  char bSpace;
  char bR;
  char bZ;
} KeyPresses_t;

static void InitializeKeyPresses(KeyPresses_t *keys);
static char CheckInput(KeyPresses_t *keys);
static char HandleInput(ThreadedLifeContext_t *context, KeyPresses_t * const keys);
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
    options.regionSize, 0, 1, options.lifeFile); //default block size of 8 for now
  if (worldContext == NULL)
  {
    printf("Failed to ThreadedLifeContext_t!\n");
    return 1;
  }

  /* TODO: Perhaps this graphics context doesn't need to be created here?
  SyncWorldToScreen already has the information required to generate this. 
  Maybe we just need an "InitLifeGraphics" function to set things up for game
  specific drawing? Or maybe SyncWorldToScreen can allocate and maintain state
  for things as needed. */
  LifeGraphicsContext_t *graphicsContext = 
    CreateLifeGraphicsContext(worldContext->front->width, worldContext->front->height);
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

    //input handling
    KeyPresses_t keys;
    InitializeKeyPresses(&keys);
    CheckInput(&keys);
    HandleInput(worldContext, &keys);
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

void InitializeKeyPresses(KeyPresses_t *keys)
{
  keys->bSpace = 0;
  keys->bEsc = 0;
  keys->bR = 0;
  keys->bZ = 0;
}

char CheckInput(KeyPresses_t *keys)
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
  }

  return 1;
}

static char HandleInput(ThreadedLifeContext_t *context, KeyPresses_t * const keys)
{
  if (keys->bZ)
  {
    SDL_LockMutex(context->lock);
    context->bRandomize = 1;
    SDL_UnlockMutex(context->lock);
  }
  if (keys->bR)
  {
    //reload the file
    SDL_LockMutex(context->lock);
    context->bReloadFile = 1;
    SDL_UnlockMutex(context->lock);
  }
  if (keys->bSpace)
  {
    //pause the simulation or unpause it.
    if (context->bSimulating)
      context->bSimulating = 0;
    else
      context->bSimulating = 1;
  }
  if (keys->bEsc)
  {
    //we have to unlock the threads by setting bSimulating to 1 otherwise
    //they will hang when we try to quit.
    context->bSimulating = 1;
    context->bRunning = 0;
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
  options->regionSize = 4; //4 seems to be the fastest. Minimum is 2, 1 breaks sim.
  options->resX = 1024;
  options->resY = 768;
  options->lifeFile[0] = '\0';
  char *cvalue = NULL;

  int c;
  while ((c = getopt(argc, argv, "l:x:y:w:h:r:fb")) != -1 )
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
      case 'r':
        cvalue = optarg;
        options->regionSize = atoi(cvalue);
        if (options->regionSize < 2)
          options->regionSize = 2;
        break;
      case 'l':
        if (strlen(optarg) <= MAX_FILENAME_LENGTH)
          sprintf(options->lifeFile, "%s", optarg);
        else
          puts("Input filename too long!");
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
