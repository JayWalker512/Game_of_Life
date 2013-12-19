#ifndef THREADLIFE_H
#define THREADLIFE_H

#include <SDL2/SDL.h>

typedef char LifeWorldCell_t;
typedef long LifeWorldDim_t;

/*Does this name really make sense/appropriate in usage? It's really just a 
buffer for simulation. Two of them are required to actually simulate a life
generation, so is "LifeWorld" a good name? 
Maybe "LifeBuffer" of "LifeWorldBuffer" */
typedef struct LifeWorld_s {
	LifeWorldCell_t *world;
	LifeWorldDim_t width;
	LifeWorldDim_t height;
} LifeWorld_t;

/* A "LifeWorld" would be the simplest unit required to simulate a generation.
ThreadWorldContext contains one of these and additional data required to 
execute in a thread? */
typedef struct ThreadWorldContext_s {
	LifeWorld_t *front;
	LifeWorld_t *back;
	SDL_mutex *lock;
	char bRunning;
	char bRandomize;
} ThreadWorldContext_t;

void *ThreadLifeMain(void *worldContext);
ThreadWorldContext_t *CreateThreadWorldContext(LifeWorldDim_t w, LifeWorldDim_t h,
		char bRunning, char bRandomize);
void DestroyThreadWorldContext(ThreadWorldContext_t *context);
LifeWorld_t *NewLifeWorld(LifeWorldDim_t width, LifeWorldDim_t height);
void DestroyLifeWorld(LifeWorld_t *world);
void SwapWorldPointers(LifeWorld_t **front, LifeWorld_t **back);
void SwapThreadWorldContextPointers(ThreadWorldContext_t *worldContext);
void CopyWorld(LifeWorld_t *dest, LifeWorld_t * const source);

LifeWorldCell_t GetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world);
void SetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world, LifeWorldCell_t);

LifeWorldCell_t SetWorldState(LifeWorld_t *world, LifeWorldCell_t state);
void RandomizeWorldStateBinary(ThreadWorldContext_t *worldContext);
void LifeGeneration(LifeWorld_t *newWorld, LifeWorld_t *const oldWorld);
unsigned long DoGensPerSec(unsigned long gens);

#endif
