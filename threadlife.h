#ifndef THREADLIFE_H
#define THREADLIFE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "dirtyregion.h"

typedef char LifeWorldCell_t;
typedef long LifeWorldDim_t;

typedef struct LifeWorldBuffer_s {
	LifeWorldCell_t *world;
	LifeWorldDim_t width;
	LifeWorldDim_t height;
} LifeWorldBuffer_t;

typedef struct LifeRules_s {
	int birthMask;
	int survivalMask;
} LifeRules_t;

typedef struct ThreadedLifeContext_s {
	char *lifeFile;
	LifeWorldBuffer_t *front;
	LifeWorldBuffer_t *back;
	DirtyRegionBuffer_t *frontRegions;
	DirtyRegionBuffer_t *backRegions;
	SDL_mutex *lock;
	LifeRules_t lifeRules;
	char bRunning;
	char bRandomize;
	char bReloadFile;
	char bSimulating;
} ThreadedLifeContext_t;

void ThreadLifeMain(void *worldContext);
ThreadedLifeContext_t *CreateThreadedLifeContext(LifeWorldDim_t w, LifeWorldDim_t h,
		LifeRules_t *lifeRules, int regionSize, char bRandomize, char bSimulating, const char *lifeFile); //pass "" or otherwise NULL to lifeFile to ignore argument.
void DestroyThreadedLifeContext(ThreadedLifeContext_t *context);
LifeWorldBuffer_t *NewLifeWorld(LifeWorldDim_t width, LifeWorldDim_t height);
void DestroyLifeWorld(LifeWorldBuffer_t *world);

void SwapWorldPointers(LifeWorldBuffer_t **front, LifeWorldBuffer_t **back); //can be static
void SwapDirtyRegionPointers(DirtyRegionBuffer_t **front, DirtyRegionBuffer_t **back);
//combines the above two functions in to one so we only need to lock once.
void SwapThreadedLifeContextGenerationPointers(ThreadedLifeContext_t *worldContext);

int CopyWorld(LifeWorldBuffer_t *dest, LifeWorldBuffer_t * const source);

LifeWorldCell_t GetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldBuffer_t *world);
void SetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldBuffer_t *world, LifeWorldCell_t state);

LifeWorldCell_t SetWorldState(LifeWorldBuffer_t *world, LifeWorldCell_t state);
void ClearWorldBuffer(LifeWorldBuffer_t *world, LifeWorldCell_t state);
void RandomizeWorldStateBinary(ThreadedLifeContext_t *worldContext);
char NumLiveNeighbors(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorldBuffer_t *world);
void LifeGeneration(LifeWorldBuffer_t *newWorld, LifeWorldBuffer_t *const oldWorld);
unsigned long DoGensPerSec(unsigned long gens);

#endif
