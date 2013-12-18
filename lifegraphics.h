#ifndef LIFEGRAPHICS_H
#define LIFEGRAPHICS_H

#include "graphics.h"
#include "threadlife.h"

typedef struct LifeGraphicsContext_s {
	LifeWorld_t *pWorldRenderBuffer;
	QuadDrawData_t *pQuadDrawData;
} LifeGraphicsContext_t;

void DrawWorld(SDL_Window *window, LifeGraphicsContext_t *graphicsContext);
void SyncWorldToScreen(SDL_Window *window, 
	ThreadWorldContext_t *worldContext, 
	LifeGraphicsContext_t *graphicsContext,
	int syncRateHz);

#endif
