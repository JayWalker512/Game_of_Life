#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "main.h"
#include "threadlife.h"
#include <SDL2/SDL.h>

void *ThreadLifeMain(void *worldContext)
{
	ThreadWorldContext_t *context = worldContext;

	unsigned long generations = 0;
	while (context->bRunning)
	{
    LifeGeneration(context->back, context->front);
    SwapThreadWorldContextPointers(context);
    generations = DoGensPerSec(generations);

    if (context->bRandomize)
    {
    	SDL_LockMutex(context->lock);
    	context->bRandomize = 0;
    	RandomizeWorldStateBinary(context);
    	SDL_UnlockMutex(context->lock);
    }
	}
	return NULL;
}

LifeWorld_t *NewLifeWorld(LifeWorldDim_t width, LifeWorldDim_t height)
{
  LifeWorld_t *world = malloc(sizeof(LifeWorld_t));
  world->world = malloc(sizeof(unsigned char) * (width * height));
  world->width = width;
  world->height = height;

  //Initialize world to dead
  unsigned int i = 0;
  for (i = 0; i < (width * height); i++)
    world->world[i] = 0;

  return world;
}

void DestroyLifeWorld(LifeWorld_t *world)
{
  free(world->world);
  free(world);
}

void SwapWorldPointers(LifeWorld_t **front, LifeWorld_t **back)
{
  LifeWorld_t *temp = *front;
  *front = *back;
  *back = temp;
}

void SwapThreadWorldContextPointers(ThreadWorldContext_t *worldContext)
{
  SDL_LockMutex(worldContext->lock);
  SwapWorldPointers(&worldContext->front, &worldContext->back);
  SDL_UnlockMutex(worldContext->lock);
}

void CopyWorld(LifeWorld_t *dest, LifeWorld_t * const source)
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

LifeWorldCell_t GetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world)
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

void SetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world, LifeWorldCell_t state)
{
  world->world[(y * world->width) + x] = state;
}

void RandomizeWorldStateBinary(ThreadWorldContext_t *worldContext)
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

char NumLiveNeighbors(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world)
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

LifeWorldCell_t SetWorldState(LifeWorld_t *world, LifeWorldCell_t state)
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

void LifeGeneration(LifeWorld_t *newWorld, LifeWorld_t *const oldWorld)
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
