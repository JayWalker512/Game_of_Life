#ifndef LIFEGRAPHICS_H
#define LIFEGRAPHICS_H

#include "graphics.h"
#include "threadlife.h"

/* Pretty much written for drawing large grids of quads, based on the
contents of pWorldRenderBuffer */
/*Do we even need these? Certainly not publicly necessary, rendering subsystem
should take care of all the low level stuff. This is unnecessarily
exposed and inflexible. */
typedef struct LifeGraphicsContext_s {
	LifeWorldBuffer_t *pWorldRenderBuffer;
	QuadDrawData_t *pQuadDrawData;
} LifeGraphicsContext_t;
LifeGraphicsContext_t *CreateLifeGraphicsContext(LifeWorldDim_t w, LifeWorldDim_t h);
void DestroyLifeGraphicsContext(LifeGraphicsContext_t *context);

/*Maybe have a "DrawQuadsFromWorld(qdd dest, lwb src)" func and then just a
"RenderQuadsToScreen(vec3 scale, vec3 translate, vec3 color) func, and finally
compose those in some DrawGameGraphics() function? We would use a common shader
that only takes the params from RenderQuadsToScreen. OR maybe the shader params
should be maintained in the qdd struct and handled at a lower level in graphics.c. 
That's probably better. Just with like a "SetQuadParams()"" func or something. */
void DrawWorld(SDL_Window *window, LifeGraphicsContext_t *graphicsContext);

/* This can do without the graphicsContext. It should remain public though as a
"do all the drawing" type call. All relevant information is in worldContext */
void SyncWorldToScreen(SDL_Window *window, 
	ThreadedLifeContext_t *worldContext, 
	LifeGraphicsContext_t *graphicsContext,
	int syncRateHz);

#endif
