#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include "graphics.h"

typedef char LifeWorldCell_t;
typedef long LifeWorldDim_t;

typedef struct LifeWorld_s {
	LifeWorldCell_t *world;
	LifeWorldDim_t width;
	LifeWorldDim_t height;
} LifeWorld_t;

typedef struct ThreadWorldContext_s {
	LifeWorld_t *front;
	LifeWorld_t *back;
	pthread_mutex_t lock;
	char bRunning;
	char bRandomize;
} ThreadWorldContext_t;

typedef struct LifeGraphicsContext_s {
	LifeWorld_t *pWorldRenderBuffer;
	QuadDrawData_t *pQuadDrawData;
} LifeGraphicsContext_t;

char CheckInput(char *bRandomizeWorld);
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
void DrawWorld(SDL_Window *window, LifeGraphicsContext_t *graphicsContext);
void SyncWorldToScreen(SDL_Window *window, 
	ThreadWorldContext_t *worldContext, 
	LifeGraphicsContext_t *graphicsContext,
	int syncRateHz);
unsigned long DoGensPerSec(unsigned long gens);

#endif
