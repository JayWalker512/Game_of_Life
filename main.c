//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc main.c threadlife.c graphics.c lifegraphics.c loadfile.c dirtyregion.c stack.c vector3.c -o gameoflife -std=c99 -Wall -Wextra -pedantic -g -D_GNU_SOURCE -I/usr/local/include/SDL2 -lSDL2 -I/usr/include/GL -lGL -lGLEW -lm
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "lifegraphics.h"
#include "loadfile.h"
#include "vector3.h"
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

static void InitializeKeyPresses(InputDeviceValues_t *keys);
static char CheckInput(InputDeviceValues_t *keys);
static char HandleInput(ThreadedLifeContext_t *worldContext, 
  LifeGameGraphicsContext_t *graphicsContext, 
  InputDeviceValues_t * const keys);
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

  LifeGameGraphicsContext_t *graphicsContext = 
    CreateLifeGameGraphicsContext(worldContext);
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
    InputDeviceValues_t keys;
    InitializeKeyPresses(&keys);
    CheckInput(&keys);
    HandleInput(worldContext, graphicsContext, &keys);
  }

  //cleaning up
  SDL_WaitThread(lifeThread, NULL);
  DestroyThreadedLifeContext(worldContext);
  DestroyLifeGameGraphicsContext(graphicsContext);
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

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

static char HandleInput(ThreadedLifeContext_t *worldContext,
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
