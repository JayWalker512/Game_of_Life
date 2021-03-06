//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc main.c threadlife.c graphics.c lifegraphics.c loadfile.c dirtyregion.c stack.c vector3.c binary.c input.c -o gameoflife -std=c99 -Wall -Wextra -pedantic -g -D_GNU_SOURCE -I/usr/local/include/SDL2 -lSDL2 -I/usr/include/GL -lGL -lGLEW -lm
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "lifegraphics.h"
#include "loadfile.h"
#include "vector3.h"
#include "binary.h"
#include "input.h"
#include <SDL2/SDL.h>

typedef struct LifeArgOptions_s {
  LifeWorldDim_t worldWidth;
  LifeWorldDim_t worldHeight;
  int resX;
  int resY;
  LifeRules_t lifeRules;
  int syncRate;
  int regionSize;
  char bFullScreen;
  char lifeFile[MAX_FILENAME_LENGTH];
  char outputFile[MAX_FILENAME_LENGTH];
  int numThreads;
	long terminationGeneration;
} LifeArgOptions_t;

static char ParseArgs(LifeArgOptions_t *options, int argc, char **argv);
static void PrintSimOptions(LifeArgOptions_t * const options, ThreadedLifeContext_t *context);
static void PrintStatsOncePerSecond(GraphicsStats_t *graphicsStats, ThreadedLifeContext_t *lifeContext);

int main(int argc, char **argv)
{
  LifeArgOptions_t options;
  ParseArgs(&options, argc, argv);

  SDL_Window *window = InitSDL(options.resX, options.resY, 
    "Game of Life", options.bFullScreen);
  SDL_GLContext glContext = InitSDL_GL(window);

  ThreadedLifeContext_t *worldContext = 
    CreateThreadedLifeContext(options.worldWidth, options.worldHeight, 
      &options.lifeRules, options.regionSize, 0, 1, options.lifeFile, 
      options.numThreads, options.terminationGeneration); //default block size of 8 for now
  if (worldContext == NULL)
  {
    printf("Failed to ThreadedLifeContext_t!\n");
    return 1;
  }
	else
		PrintSimOptions(&options, worldContext);

  LifeGameGraphicsContext_t *graphicsContext = 
    CreateLifeGameGraphicsContext(worldContext);
  if (graphicsContext == NULL)
  {
    printf("Failed to create LifeGraphicsContext_t!\n");
    return 1;
  }

  //create and start the simulation thread(s).
  SDL_Thread *lifeThread[options.numThreads]; 
  for (int i = 0; i < options.numThreads; i++)
  {
    char threadTitle[10] = "SimThread."; //. is the spot where our ID goes
    threadTitle[9] = i + 65;
    lifeThread[i] = SDL_CreateThread((SDL_ThreadFunction)&ThreadLifeMain, 
      threadTitle, worldContext);
    if (lifeThread[i] == NULL)
    {
      printf("Couldn't create thread!\n");
      return 1;
    }
  }

  InputDeviceValues_t keys;
  InitializeKeyPresses(&keys);
  GraphicsStats_t graphicsStats;
  InitializeGraphicsStats(&graphicsStats);
  UpdateGraphicsStats(&graphicsStats); //to avoid printing FPS: 0 twice
  //we start this loop in main thread that only does rendering and input
  while (worldContext->bRunning)
  {
    //Draw world so we can see initial state. 
    if(SyncWorldToScreen(window, worldContext, graphicsContext, options.syncRate))
      UpdateGraphicsStats(&graphicsStats);

    //input handling
    CheckInput(&keys);
    HandleInput(worldContext, graphicsContext, &keys);
    PrintStatsOncePerSecond(&graphicsStats, worldContext);
  }
  //cleaning up
  for (int i = 0; i < options.numThreads; i++)
    SDL_WaitThread(lifeThread[i], NULL);

  if (options.outputFile[0] != '\0')
    SaveLifeWorld(options.outputFile, worldContext->front);

  DestroyThreadedLifeContext(worldContext);
  DestroyLifeGameGraphicsContext(graphicsContext);
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

char ParseArgs(LifeArgOptions_t *options, int argc, char **argv)
{
  //set default options first, then parse for changes.
  options->bFullScreen = 0;
  options->worldWidth = 400;
  options->worldHeight = 300;
  options->regionSize = 4; //4 seems to be the fastest. Minimum is 2, 1 breaks sim.
  options->resX = 1024;
  options->resY = 768;
  options->lifeFile[0] = '\0';
  options->outputFile[0] = '\0';
  options->lifeRules.birthMask = 8;
  options->lifeRules.survivalMask = 12;
  options->syncRate = 60;
  options->numThreads = 1; //1 simulation thread. graphics/main thread is implied.
	options->terminationGeneration = -1;
  char *cvalue = NULL;

  int c;
  while ((c = getopt(argc, argv, "l:x:y:w:h:r:b:s:z:t:g:o:f")) != -1 )
  {
    switch (c)
    {
      case 'f':
        options->bFullScreen = 1;
        break;
      case 'b':
        cvalue = optarg;
        options->lifeRules.birthMask = GetMaskFromStringInt(cvalue);
        break;
      case 's':
        cvalue = optarg;
        options->lifeRules.survivalMask = GetMaskFromStringInt(cvalue);
        break;
      case 'x':
        cvalue = optarg;
        options->resX = strtol(cvalue, NULL, 10);
        break;
      case 'y':
        cvalue = optarg;
        options->resY = strtol(cvalue, NULL, 10);
        break;
      case 'w':
        cvalue = optarg;
        options->worldWidth = strtol(cvalue, NULL, 10);
        break;
      case 'h':
        cvalue = optarg;
        options->worldHeight = strtol(cvalue, NULL, 10);
        break;
      case 'r':
        cvalue = optarg;
        options->regionSize = strtol(cvalue, NULL, 10);
        if (options->regionSize < 2)
          options->regionSize = 2;
        break;
			case 'g':
        cvalue = optarg;
        options->terminationGeneration = strtol(cvalue, NULL, 10);
				//negative values here won't break anything, so no reason to check. Maybe use unsigned val and make 0 the unlimited marker?
        break;
      case 'o':
        if (strlen(optarg) <= MAX_FILENAME_LENGTH)
          sprintf(options->outputFile, "%s", optarg);
        else
          puts("Output filename too long!");
        break;
      case 'z': //sets the SyncToScreen rate.
        cvalue = optarg;
        int rate = strtol(cvalue, NULL, 10);
        if (rate < 0)
          rate = 0;
        options->syncRate = rate;
        break;
      case 't': //sets number of sim threads SPAWNED. 1 is minimum value to input here.
        cvalue = optarg;
        int threads = strtol(cvalue, NULL, 10);
        threads -= 1; //ex. user inputs "2" for 2 total threads, but only 1 is a sim thread
        if (threads < 1)
          threads = 1;
        options->numThreads = threads;
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
        break;
      default:
        puts("What's going on in here?!?!");
        break;
    }
  }
  return 1;
}

static void PrintSimOptions(LifeArgOptions_t * const options, ThreadedLifeContext_t *context)
{
	printf("Options:\n");
	if (options->bFullScreen)
		printf("Window: %dx%d Fullscreen\n", options->resX, options->resY);
	else
		printf("Window: %dx%d\n", options->resX, options->resY);

	printf("World size: %dx%d\n", (int)context->front->width, (int)context->front->height);
	printf("Region size: %d\n", options->regionSize);
	printf("Sync rate: %d\n", options->syncRate);
	printf("Threads: %d\n", options->numThreads + 1);
	if (context->lifeFile[0] != '\0')
		printf("Loaded file: %s\n", context->lifeFile);
}

static void PrintStatsOncePerSecond(GraphicsStats_t *graphicsStats, ThreadedLifeContext_t *lifeContext)
{
  static long endTime = 0;
  long now = SDL_GetTicks();
  if (now > endTime)
  {
		endTime = now + 1000;
		long gensPerSec, generation = 0;
		GetLifeStats(&generation, &gensPerSec, lifeContext);
    //do per-second stuff here
    printf("Frames/s: %ld, Gens/s: %ld, Generation: %ld\n", 
			graphicsStats->fps, gensPerSec, generation);

  }
}
