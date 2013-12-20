#ifndef LIFEGRAPHICS_H
#define LIFEGRAPHICS_H

#include "graphics.h"
#include "threadlife.h"

typedef struct LifeGraphicsContext_s {
	LifeWorldBuffer_t *pWorldRenderBuffer;
	QuadDrawData_t *pQuadDrawData;
} LifeGraphicsContext_t;

LifeGraphicsContext_t *CreateLifeGraphicsContext(LifeWorldDim_t w, LifeWorldDim_t h,
		const char *vertexShaderPath, const char *fragmentShaderPath);
void DestroyLifeGraphicsContext(LifeGraphicsContext_t *context);

void DrawWorld(SDL_Window *window, LifeGraphicsContext_t *graphicsContext);
void SyncWorldToScreen(SDL_Window *window, 
	ThreadedLifeContext_t *worldContext, 
	LifeGraphicsContext_t *graphicsContext,
	int syncRateHz);

#endif
