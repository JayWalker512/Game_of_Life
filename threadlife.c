#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "loadfile.h"
#include "stack.h"
#include "binary.h"

/* STATIC FUNCTIONS DECLARED AND DEFINED HERE! */

static void BuildSimBlockQueues(Stack_t *simBlockQueue, DirtyRegionBuffer_t *dirtyRegions)
{
  for (int i = 0; i < NumRegions(dirtyRegions); i++)
  {
    if (GetRegionValue(dirtyRegions, i))
      StackPush(simBlockQueue, i);
  }
}

static void CopyWorldBlock(LifeWorldBuffer_t *dest, LifeWorldBuffer_t *src, int x, int y, int w, int h)
{
  /*Much slower than doing a lower level memory->memory copy, but easier to read... */
  for (int iterY = y; iterY < y + h; iterY++)
  {
    for (int iterX = x; iterX < x + w; iterX++)
    {
      if (GetCellState(iterX, iterY, src))
        SetCellState(iterX, iterY, dest, 1);
      else
        SetCellState(iterX, iterY, dest, 0);
    }
  }
}

static int CopyWorldBlocksFromQueue(LifeWorldBuffer_t *front, LifeWorldBuffer_t *back, 
  DirtyRegionBuffer_t * const dirtyRegionsReference, Stack_t *copyBlockQueue)
{
  int blocksCopied = 0; //we return this value from this func
  while(!StackIsEmpty(copyBlockQueue))
  {
    int region = StackPop(copyBlockQueue);
    int x = 0;
    int y = 0;
    int dim = 0;
    GetRegionSourceDims(dirtyRegionsReference, region, &x, &y, &dim, &dim);
    //printf("Copying region %d at %d,%d being %d dimensions\n", region, x, y, dim);
    CopyWorldBlock(front, back, x, y, dim, dim);
    blocksCopied++;
  }
  return blocksCopied;
}

static void ClearWorldBlock(LifeWorldBuffer_t *world, LifeWorldDim_t x, LifeWorldDim_t y,
  LifeWorldDim_t w, LifeWorldDim_t h, LifeWorldCell_t state)
{
  for (int iterY = y; iterY < y + h; iterY++)
  {
    for (int iterX = x; iterX < x + w; iterX++)
    {
      SetCellState(iterX, iterY, world, state);
    }
  }
}

static void MarkAffectedRegions(LifeWorldDim_t x, LifeWorldDim_t y, DirtyRegionBuffer_t *dirtyBuffer)
{
  MarkRegion(dirtyBuffer, x, y, 1);
  MarkRegion(dirtyBuffer, x - 1, y - 1, 1); //upper left
  MarkRegion(dirtyBuffer, x + 1, y - 1, 1); //upper right
  MarkRegion(dirtyBuffer, x - 1, y + 1, 1); //lower left
  MarkRegion(dirtyBuffer, x + 1, y + 1, 1); //lower right
  /* Marking regions this way ensures that we mark any adjacent regions without
  doing any special checks. */
}

static void SimWorldBlock(LifeWorldBuffer_t *back, DirtyRegionBuffer_t *backRegions,
  LifeWorldBuffer_t *front, LifeRules_t *lifeRules,
  LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldDim_t w, LifeWorldDim_t h)
{
  //If the block didn't change state at all during simulation, we copy it to the
  //opposite buffer so it will remain static on each buffer flip.
  char bBlockChangedState = 0;  

  //Clear the block we're about to write our simulation result to.
  //We don't really need to do this... right? The whole block of the back buffer should be written to.
  //ClearWorldBlock(back, x, y, w, h, 0);

  LifeWorldDim_t iterX = 0;
  LifeWorldDim_t iterY = 0;
  for (iterY = y; iterY < y + h; iterY++)
  {
    for (iterX = x; iterX < x + w; iterX++)
    {
      int numNeighbors = NumLiveNeighbors(iterX, iterY, front);
      int cellIsLiving = GetCellState(iterX, iterY, front);

      int neighborsMask = 0;
      SetBitInt(&neighborsMask, numNeighbors);

      if (cellIsLiving)
      {
        if (neighborsMask & lifeRules->survivalMask)
        {
          SetCellState(iterX, iterY, back, 1); //cell was alive, stays alive
          //need not mark regions if nothing changed.
        }
        else if (neighborsMask & ~(lifeRules->survivalMask))
        {
          SetCellState(iterX, iterY, back, 0);
          MarkAffectedRegions(iterX, iterY, backRegions);
          bBlockChangedState = 1;
        }
      }

      if (!cellIsLiving && (neighborsMask & lifeRules->birthMask)) //dead cell becomes living
      {
        SetCellState(iterX, iterY, back, 1);
        MarkAffectedRegions(iterX, iterY, backRegions);
        bBlockChangedState = 1;
      }
      else if (!cellIsLiving && (neighborsMask & ~(lifeRules->birthMask)))
      {
        SetCellState(iterX, iterY, back, 0); 
        //Cell stays dead. We must make sure we write to every cell in the new
        //buffer whether it changes or not, to overwrite the previous state it contained.
        //This is to avoid having to clear each destination block prior to simulation. 
      }
    }
  }

  if (!bBlockChangedState)
  {
    CopyWorldBlock(back, front, x, y, w, h);
  }
}

static void SimWorldBlocksFromQueue(LifeWorldBuffer_t *back, DirtyRegionBuffer_t *backRegions, 
        LifeWorldBuffer_t *front, DirtyRegionBuffer_t *frontRegions, Stack_t *simBlockQueue,
        LifeRules_t *lifeRules)
{
  while (!StackIsEmpty(simBlockQueue))
  {
    int x = 0;
    int y = 0;
    int dim = 0;
    int region = StackPop(simBlockQueue);
    GetRegionSourceDims(frontRegions, region, &x, &y, &dim, &dim);
    SimWorldBlock(back, backRegions, front, lifeRules, x, y, dim, dim);
  }
}

/* PUBLIC FUNCTIONS BEGIN HERE */
/*
void ThreadLifeMain(void *worldContext)
{
  ThreadedLifeContext_t *context = worldContext;

  Stack_t *simBlockQueue = NewStack(NumRegions(context->frontRegions));

  unsigned long generations = 0;
  while (context->bRunning)
  {
    //essentially all the stuff below should only take place during a buffer swap

    ClearDirtyRegionBuffer(context->backRegions, 0); //0 means copy, 1 means simulate
    BuildSimBlockQueues(simBlockQueue, context->frontRegions);
    SimWorldBlocksFromQueue(context->back, context->backRegions, 
        context->front, context->frontRegions, simBlockQueue, 
        &context->lifeRules);

    //renamed version of SwapThreadedLifeContextPointers()
    SwapThreadedLifeContextGenerationPointers(context);
    generations = DoGensPerSec(generations);

    //delay each simulation frame if necessary.
    SDL_Delay(context->generationDelayMs);

    while (!context->bSimulating)
      SDL_Delay(10);

    SDL_LockMutex(context->lock);
    if (context->bRandomize)
    {
      context->bRandomize = 0;
      RandomizeWorldStateBinary(context); //this isn't a very good func name/setup...
      ClearDirtyRegionBuffer(context->frontRegions, 1);
    }

    if (context->bReloadFile)
    {
      context->bReloadFile = 0;
      ClearWorldBuffer(context->front, 0);
      LoadLifeWorld(context->front, context->lifeFile, 1);
      ClearDirtyRegionBuffer(context->frontRegions, 1);
    }

    if (context->bClearWorld)
    {
      context->bClearWorld = 0;
      ClearWorldBuffer(context->front, 0);
      ClearWorldBuffer(context->back, 0);
      ClearDirtyRegionBuffer(context->frontRegions, 1);
    }
    SDL_UnlockMutex(context->lock);

  }

  DestroyStack(simBlockQueue);
}
*/

void ThreadLifeMain(void *worldContext)
{
  ThreadedLifeContext_t *context = worldContext;

  const int localQueueSize = 8;
  Stack_t *localSimBlockQueue = NewStack(localQueueSize);

  while (context->bRunning)
  {
    //fill work queue if available.
    SDL_LockMutex(context->lock);
    if (StackIsEmpty(context->simBlockQueue))
    {
      /*We could use a counter that each thread adds to when it completes its work.
      When the counter reaches (numthreads - 1), we know that WE are the last 
      thread to finish, and can sync/swap pointers.*/
      
      //we got here, stack is empty, we've completed our work!
      //are we the last to finish?
      //yes:
      //  swap buffers, fill next queue
      //no:
      //  do nothing. locks+stack empty checks should take care of the rest.

      context->numThreadsCompletedWork++; //XXX same thread can increment this multiple times?
      if (context->numThreadsCompletedWork == context->numThreads)
      {
        SwapThreadedLifeContextGenerationPointers(context);

        if (context->bRandomize)
        {
          context->bRandomize = 0;
          RandomizeWorldStateBinary(context); //this isn't a very good func name/setup...
          ClearDirtyRegionBuffer(context->frontRegions, 1);
        }

        if (context->bReloadFile)
        {
          context->bReloadFile = 0;
          ClearWorldBuffer(context->front, 0);
          LoadLifeWorld(context->front, context->lifeFile, 1);
          ClearDirtyRegionBuffer(context->frontRegions, 1);
        }

        if (context->bClearWorld)
        {
          context->bClearWorld = 0;
          ClearWorldBuffer(context->front, 0);
          ClearWorldBuffer(context->back, 0);
          ClearDirtyRegionBuffer(context->frontRegions, 1);
        }

        ClearDirtyRegionBuffer(context->backRegions, 0); //0 means null, 1 means simulate
        BuildSimBlockQueues(context->simBlockQueue, context->frontRegions);

        context->numThreadsCompletedWork = 0;
      }
    }
    else
    {
      for (int i = 0; i < localQueueSize; i++)
      {
        if (!StackIsEmpty(context->simBlockQueue))
        {
          StackPush(localSimBlockQueue, StackPop(context->simBlockQueue));
        }
        else
          i = localQueueSize;
      }
    }
    //XXX: unlock SHOULD go here, but there's a race condition somewhere

    if (!StackIsEmpty(localSimBlockQueue))
    {
      SimWorldBlocksFromQueue(context->back, context->backRegions, 
        context->front, context->frontRegions, localSimBlockQueue, 
        &context->lifeRules);
    }
    SDL_UnlockMutex(context->lock);

    while (!context->bSimulating)
      SDL_Delay(10);
  }

  DestroyStack(localSimBlockQueue);
}

ThreadedLifeContext_t *CreateThreadedLifeContext(LifeWorldDim_t w, LifeWorldDim_t h,
    LifeRules_t *lifeRules, int regionSize, char bRandomize, char bSimulating, 
    const char *lifeFile, int numThreads)
{
  ThreadedLifeContext_t *context = malloc(sizeof(ThreadedLifeContext_t));

  LifeWorldDim_t scaledWidth = w;
  LifeWorldDim_t scaledHeight = h;

  while (scaledWidth % regionSize != 0)
    scaledWidth++;

  while (scaledHeight % regionSize != 0)
    scaledHeight++;

  //w and h MUST be multples of regionSquareDims for things to work!
  context->front = NewLifeWorld(scaledWidth, scaledHeight);
  context->back = NewLifeWorld(scaledWidth, scaledHeight);
  context->frontRegions = NewDirtyRegionBuffer(regionSize, scaledWidth, scaledHeight);
  context->backRegions = NewDirtyRegionBuffer(regionSize, scaledWidth, scaledHeight);

  //init front to dirty so we must simulate ALL cells first gen
  ClearDirtyRegionBuffer(context->frontRegions, 1); 
  ClearDirtyRegionBuffer(context->backRegions, 0);

  context->lock = SDL_CreateMutex();
  if (!context->lock)
  {
    printf("Failed to create ThreadedLifeContext.lock!\n");
    DestroyThreadedLifeContext(context);
    return NULL;
  }

  if (bRandomize)
    RandomizeWorldStateBinary(context);

  context->lifeFile = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
  context->lifeFile[0] = '\0'; 
  if (lifeFile[0] != '\0')
  {
    if (LoadLifeWorld(context->front, lifeFile, 1) == 0)
    {
      printf("Failed to load file!\n");
    } 
    else
    {
      //maybe LoadLifeWorld should point to a context to fill this automatically?
      snprintf(context->lifeFile, MAX_FILENAME_LENGTH, "%s", lifeFile);
    }
  }

  context->lifeRules.birthMask = lifeRules->birthMask;
  context->lifeRules.survivalMask = lifeRules->survivalMask;

  context->simBlockQueue = NewStack(NumRegions(context->frontRegions));
  BuildSimBlockQueues(context->simBlockQueue, context->frontRegions);
  context->numThreads = numThreads;
  context->numThreadsCompletedWork = 0;
 
  context->generationDelayMs = 0;
  context->bReloadFile = 0;
  context->bClearWorld = 0;
  context->bRandomize = 0; //we only want to randomize once from here or not at all
  context->bSimulating = bSimulating;
  context->bRunning = 1; //if we're not running, the program is quitting
  return context;
}

void DestroyThreadedLifeContext(ThreadedLifeContext_t *context)
{
  free(context->lifeFile);
  DestroyLifeWorld(context->front);
  DestroyLifeWorld(context->back);
  DestroyDirtyRegionBuffer(context->frontRegions);
  DestroyDirtyRegionBuffer(context->backRegions);
  DestroyStack(context->simBlockQueue);
  SDL_DestroyMutex(context->lock);
  free(context);
}

LifeWorldBuffer_t *NewLifeWorld(LifeWorldDim_t width, LifeWorldDim_t height)
{
  LifeWorldBuffer_t *world = malloc(sizeof(LifeWorldBuffer_t));
  world->world = malloc(sizeof(LifeWorldCell_t) * (width * height));
  world->width = width;
  world->height = height;

  //Initialize world to dead
  unsigned int i = 0;
  for (i = 0; i < (width * height); i++)
    world->world[i] = 0;

  return world;
}

void DestroyLifeWorld(LifeWorldBuffer_t *world)
{
  free(world->world);
  free(world);
}

void SwapWorldPointers(LifeWorldBuffer_t **front, LifeWorldBuffer_t **back)
{
  LifeWorldBuffer_t *temp = *front;
  *front = *back;
  *back = temp;
}

void SwapDirtyRegionPointers(DirtyRegionBuffer_t **front, DirtyRegionBuffer_t **back)
{
  DirtyRegionBuffer_t *temp = *front;
  *front = *back;
  *back = temp;
}

void SwapThreadedLifeContextGenerationPointers(ThreadedLifeContext_t *worldContext)
{
  SDL_LockMutex(worldContext->lock);
  SwapWorldPointers(&worldContext->front, &worldContext->back);
  SwapDirtyRegionPointers(&worldContext->frontRegions, &worldContext->backRegions);
  SDL_UnlockMutex(worldContext->lock);
}

int CopyWorld(LifeWorldBuffer_t *dest, LifeWorldBuffer_t * const source)
{
  //when we implement threading, this will have to lock the source & dest
	//^this is done by the calling function.
  //returns 1 on success, 0 on fail.

  if (dest->width != source->width)
    return 0;

  if (dest->height != source->height)
    return 0;

  for (int i = 0; i < (dest->width * dest->height); i++)
  {
    dest->world[i] = source->world[i];
  }
  return 1;
}

LifeWorldCell_t GetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldBuffer_t *world)
{
  LifeWorldDim_t wrappedX = x; 
  LifeWorldDim_t wrappedY = y; 
	while (wrappedX >= world->width)
		wrappedX -= world->width;

	while (wrappedX < 0)
		wrappedX += world->width;

	while (wrappedY >= world->height)
		wrappedY -= world->height;

	while (wrappedY < 0)
		wrappedY += world->height;

  return (world->world[(wrappedY * world->width) + wrappedX]);
}

void SetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldBuffer_t *world, LifeWorldCell_t state)
{
  //Can easily cause memory corruption if incorrect params are passed in...
  //Maybe it should implement wrapping?
  world->world[(y * world->width) + x] = state;
}

void ClearWorldBuffer(LifeWorldBuffer_t *world, LifeWorldCell_t state)
{
  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < world->height; y++)
  {
    for (x = 0; x < world->width; x++)
    {
      SetCellState(x, y, world, state); //set to zero first so no leftover state
    }
  }
}

void RandomizeWorldStateBinary(ThreadedLifeContext_t *worldContext)
{
  /*XXX: Note that rand() and srand() are NOT thread safe! It shouldn't break anything,
    but there is hidden state in use. Will affect random number generation in OTHER threads
    when used! */
  srand(SDL_GetTicks());

  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < worldContext->front->height; y++)
  {
    for (x = 0; x < worldContext->front->width; x++)
    {
      SetCellState(x, y, worldContext->front, 0); //set to zero first so no leftover state
      SetCellState(x, y, worldContext->front, (LifeWorldCell_t)(rand() % 2));
    }
  }
}

char NumLiveNeighbors(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldBuffer_t *world)
{
  char numLive = 0;

  //up left
  if (GetCellState(x - 1, y - 1, world))
    numLive++;

  //up
  if (GetCellState(x, y - 1, world))
    numLive++;

  //up-right
  if (GetCellState(x + 1, y - 1, world))
    numLive++;

  //left
  if (GetCellState(x - 1, y, world))
    numLive++;

  //right
  if (GetCellState(x + 1, y, world))
    numLive++;

  //down-left
  if (GetCellState(x - 1, y + 1, world))
    numLive++;

  //down
  if (GetCellState(x, y + 1, world))
    numLive++;

  //down-right
  if (GetCellState(x + 1, y + 1, world))
    numLive++;

  return numLive;
}

LifeWorldCell_t SetWorldState(LifeWorldBuffer_t *world, LifeWorldCell_t state)
{
  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < world->height; y++)
  {
    for (x = 0; x < world->width; x++)
    {
      SetCellState(x, y, world, state);
    }
  }
  return state;
}

unsigned long DoGensPerSec(unsigned long gens)
{
  static unsigned long endTime = 0;
  if (SDL_GetTicks() > endTime)
  {
    endTime = SDL_GetTicks() + 1000;
    printf("Gens/s: %ld\n", gens);
    return 0;
  }
  return ++gens;
}
