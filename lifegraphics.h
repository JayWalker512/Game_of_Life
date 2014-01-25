#ifndef LIFEGRAPHICS_H
#define LIFEGRAPHICS_H

#include "graphics.h"
#include "threadlife.h"
#include "vector3.h"

typedef struct GraphicsStats_s {
	long frames;
	long fps;
} GraphicsStats_t;

typedef struct LifeGameGraphicsContext_s {
	LifeWorldBuffer_t *pWorldRenderBuffer;
	LifeWorldBuffer_t *pRegionRenderBuffer;
	QuadDrawData_t *pCellDrawData;
	QuadDrawData_t *pRegionDrawData;
	QuadDrawData_t *pBackgroundDrawData;
	Vector3_t scale;
	Vector3_t translation;
	char bDrawRegions;
} LifeGameGraphicsContext_t;

LifeGameGraphicsContext_t *CreateLifeGameGraphicsContext(ThreadedLifeContext_t *context);
void DestroyLifeGameGraphicsContext(LifeGameGraphicsContext_t *context);
void GetGameGraphicsTranslation(Vector3_t *translation, LifeGameGraphicsContext_t *const context);
void SetGameGraphicsTranslation(LifeGameGraphicsContext_t *context, Vector3_t *const translation);
void GetGameGraphicsScale(Vector3_t *scale, LifeGameGraphicsContext_t *const context);
void SetGameGraphicsScale(LifeGameGraphicsContext_t *context, Vector3_t *const scale);
void SetCellDrawColor(LifeGameGraphicsContext_t *context, Vector3_t *const rgb);
void SetRegionDrawColor(LifeGameGraphicsContext_t *context, Vector3_t *const rgb);
void SetBackgroundDrawColor(LifeGameGraphicsContext_t *context, Vector3_t *const rgb);
void DrawRegionsEnabled(LifeGameGraphicsContext_t *context, int enabled);

void DrawWorld(SDL_Window *window, LifeGameGraphicsContext_t *graphicsContext);

void SyncWorldToScreen(SDL_Window *window, 
	ThreadedLifeContext_t *worldContext, 
	LifeGameGraphicsContext_t *graphicsContext,
	int syncRateHz);

#endif
