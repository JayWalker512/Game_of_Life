#ifndef LIFEGRAPHICS_H
#define LIFEGRAPHICS_H

#include "graphics.h"
#include "threadlife.h"
#include "vector3.h"

/* as outline in my todo from 12/30/13 */
typedef struct LifeGameGraphicsContext_s {
	//I guess these two bits kinda ARE necessary...
	LifeWorldBuffer_t *pWorldRenderBuffer;
	LifeWorldBuffer_t *pRegionRenderBuffer;

	QuadDrawData_t *pCellDrawData;
	QuadDrawData_t *pRegionDrawData;
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
void DrawRegionsEnabled(LifeGameGraphicsContext_t *context, int enabled);
/* static to-be*/ void DrawGame(SDL_Window *window, LifeGameGraphicsContext_t *context);

/* OR maybe the shader params
should be maintained in the qdd struct and handled at a lower level in graphics.c. 
That's probably better. Just with like a "SetQuadParams()"" func or something. */
void DrawWorld(SDL_Window *window, LifeGameGraphicsContext_t *graphicsContext);

/* This can do without the graphicsContext. It should remain public though as a
"do all the drawing" type call. All relevant information is in worldContext */
void SyncWorldToScreen(SDL_Window *window, 
	ThreadedLifeContext_t *worldContext, 
	LifeGameGraphicsContext_t *graphicsContext,
	int syncRateHz);

#endif
