#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include "loadfile.h"
#include "stack.h"

/* STATIC FUNCTIONS DECLARED AND DEFINED HERE! */

static void BuildCopyAndSimBlockQueues(Stack_t *copyBlockQueue, Stack_t *simBlockQueue, DirtyRegionBuffer_t *dirtyRegions)
{
  for (int i = 0; i < NumRegions(dirtyRegions); i++)
  {
    if (GetRegionValue(dirtyRegions, i))
      StackPush(simBlockQueue, i);
    else
      StackPush(copyBlockQueue, i);
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
    printf("Copying region %d at %d,%d being %d dimensions\n", region, x, y, dim);
    CopyWorldBlock(front, back, x, y, dim, dim);
    blocksCopied++;
  }
  return blocksCopied;
}

/* PUBLIC FUNCTIONS BEGIN HERE */

#if 0
void ThreadLifeMain(void *worldContext)
{
	ThreadedLifeContext_t *context = worldContext;

	unsigned long generations = 0;
	while (context->bRunning)
	{
    LifeGeneration(context->back, context->front);
    SwapThreadedLifeContextPointers(context);
    generations = DoGensPerSec(generations);

    /* Should these be atomic check/changes? If two threads see a randomize/reload request
    and one locks, the other will wait to lock until the other finishes and then repeat
    the same operation. Fine with one thread, not with more. */
    if (context->bRandomize)
    {
    	SDL_LockMutex(context->lock);
    	context->bRandomize = 0;
    	RandomizeWorldStateBinary(context);
    	SDL_UnlockMutex(context->lock);
    }

    if (context->bReloadFile)
    {
      SDL_LockMutex(context->lock);
      context->bReloadFile = 0;
      ClearWorldBuffer(context->front, 0);
      LoadLifeWorld(context->front, context->lifeFile, 1);
      SDL_UnlockMutex(context->lock);
    }

    /*Quick an dirty way to pause simulation. 
    Maybe there's a better way with semaphores or something that won't peg
    all the cores while doing nothing?*/
    while (!context->bSimulating);

	}
}
#endif

void ThreadLifeMain(void *worldContext)
{
  ThreadedLifeContext_t *context = worldContext;

  Stack_t copyBlockQueue;
  Stack_t simBlockQueue;
  StackInit(&copyBlockQueue, NumRegions(context->frontRegions));
  StackInit(&simBlockQueue, NumRegions(context->frontRegions));

  unsigned long generations = 0;
  while (context->bRunning)
  {
    /*Might need to implement a stack interface here. Would be less efficient
    with more threads accessing concurrently? Could at least be used to build 
    individual stacks for each thread. */

    /*Analyzes block states in frontRegions and adds to relevant stacks based on
    block states. This might be more sensical to divide into two functions, but 
    would be slower. Why do in two passes what we can in one? */
    /* These queues SHOULD be empty when we get back here, removing need to clear
    them. Perhaps there should be more error checking/robustness involved.*/
    /* Do we even need to explicitly copy blocks that aren't being simulated? 
    Won't they be the same anyway? */
    BuildCopyAndSimBlockQueues(&copyBlockQueue, &simBlockQueue, context->frontRegions);
    ClearDirtyRegionBuffer(context->backRegions, 0);

    /* Copies blocks in front to back based on copyBlockQueue */
    CopyWorldBlocksFromQueue(context->back, context->front, 
        context->frontRegions, &copyBlockQueue);
    
    /*SimWorldBlocksFromQueue(context->back, context->backRegions, 
        context->front, context->frontRegions, &simBlockQueue);*/

    //renamed version of SwapThreadedLifeContextPointers()
    SwapThreadedLifeContextWorldBufferPointers(context); //locks are MAD slow
    SwapThreadedLifeContextDirtyRegionPointers(context);

    generations = DoGensPerSec(generations);
  }

  DestroyStack(&copyBlockQueue);
  DestroyStack(&simBlockQueue);
}

ThreadedLifeContext_t *CreateThreadedLifeContext(LifeWorldDim_t w, LifeWorldDim_t h,
    int regionSize, char bRandomize, char bSimulating, const char *lifeFile)
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
 
  context->bReloadFile = 0;
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

void SwapThreadedLifeContextWorldBufferPointers(ThreadedLifeContext_t *worldContext)
{
  SDL_LockMutex(worldContext->lock);
  SwapWorldPointers(&worldContext->front, &worldContext->back);
  SDL_UnlockMutex(worldContext->lock);
}

void SwapThreadedLifeContextDirtyRegionPointers(ThreadedLifeContext_t *worldContext)
{
  SDL_LockMutex(worldContext->lock);
  SwapDirtyRegionPointers(&worldContext->frontRegions, &worldContext->backRegions);
  SDL_UnlockMutex(worldContext->lock);
}

void CopyWorld(LifeWorldBuffer_t *dest, LifeWorldBuffer_t * const source)
{
  //when we implement threading, this will have to lock the source & dest
	//^this is done by the calling function.
  assert(source->width == dest->width);
  assert(source->height == dest->height);

  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < source->height; y++)
  {
    for (x = 0; x < source->width; x++)
    {
      SetCellState(x, y, dest, GetCellState(x, y, source));
    }
  }
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

void LifeGeneration(LifeWorldBuffer_t *newWorld, LifeWorldBuffer_t *const oldWorld)
{
  SetWorldState(newWorld, 0); //initialize newWorld before we simulate

  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < oldWorld->height; y++)
  {
    for (x = 0; x < oldWorld->width; x++)
    {
      char numNeighbors = NumLiveNeighbors(x, y, oldWorld);
      char cellIsLiving = GetCellState(x, y, oldWorld);
      if (cellIsLiving)
      {
        if (numNeighbors >= 2 && numNeighbors <= 3)
        {
          SetCellState(x, y, newWorld, 1); //cell was alive, stays alive
        } 
        else if (numNeighbors < 2 || numNeighbors > 3)
        {
          SetCellState(x, y, newWorld, 0); //cell was alive, dies
        }
      }
      
      if (!cellIsLiving && numNeighbors == 3) //cell was dead and has 3 live neighbors
      {
        SetCellState(x,y,newWorld,1); //living cell is born
      }
    }
  }
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
