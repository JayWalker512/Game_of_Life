#include "lifegraphics.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

/* Ideally there wouldn't be any GL calls or low-level backend type stuff here.
Just high level draw calls. Low level stuff should be compartmentalized in
graphics.c or alike. */

LifeGameGraphicsContext_t *CreateLifeGameGraphicsContext(ThreadedLifeContext_t *lifeContext)
{
  LifeGameGraphicsContext_t *graphicsContext = malloc(sizeof(LifeGameGraphicsContext_t));

  int worldFrontBufferWidth = lifeContext->front->width;
  int worldFrontBufferHeight = lifeContext->front->height;
  int regionDims = lifeContext->frontRegions->regionSquareDims;
  graphicsContext->pWorldRenderBuffer = NewLifeWorld(worldFrontBufferWidth, worldFrontBufferHeight);
  graphicsContext->pRegionRenderBuffer = NewLifeWorld(worldFrontBufferWidth / regionDims,
      worldFrontBufferHeight / regionDims);

  graphicsContext->pCellDrawData = NewQuadDataBuffer(worldFrontBufferWidth *worldFrontBufferHeight);
  graphicsContext->pRegionDrawData = NewQuadDataBuffer((worldFrontBufferWidth / regionDims) *
      (worldFrontBufferHeight / regionDims));

  Vector3Set(&graphicsContext->scale, 1.0, 1.0, 1.0);
  Vector3Set(&graphicsContext->translation, 0.0, 0.0, 0.0);
  char bDrawRegions = 0;

  return graphicsContext;

  /* TODO: maybe have some kind of shader-manager so that we don't need to build the
  same shader multiple times and can just reference it dynamically? */
}

void DestroyLifeGameGraphicsContext(LifeGameGraphicsContext_t *context)
{
  DestroyLifeWorld(context->pWorldRenderBuffer);
  DestroyLifeWorld(context->pRegionRenderBuffer);
  DestroyQuadDrawData(context->pCellDrawData);
  DestroyQuadDrawData(context->pRegionDrawData); //this call destroys shaders too right?
  free(context);
}

void SyncWorldToScreen(SDL_Window *window, ThreadedLifeContext_t *worldContext, LifeGameGraphicsContext_t *graphicsContext, int syncRateHz)
{
  static unsigned long endTime = 0;
  if (SDL_GetTicks() > endTime)
  {
		//Critical section!
    SDL_LockMutex(worldContext->lock);
    CopyWorld(graphicsContext->pWorldRenderBuffer, worldContext->front);
    SDL_UnlockMutex(worldContext->lock);
    DrawWorld(window, graphicsContext);
		//end crititcal section!

		if (syncRateHz > 0)
		{
    	unsigned long delayMs = 1000 / syncRateHz;
    	endTime = SDL_GetTicks() + delayMs;
		}
  }

}

void DrawWorld(SDL_Window *window, LifeGameGraphicsContext_t *graphicsContext)
{
  ClearScreen(0.1, 0.0, 0.0);
  int worldW, worldH;
  worldW = graphicsContext->pWorldRenderBuffer->width;
  worldH =  graphicsContext->pWorldRenderBuffer->height;
  int screenW, screenH;
  SDL_GetWindowSize(window, &screenW, &screenH);
  float cellWidth = 1.0f / (float)screenW * ((float)screenW / worldW);
  float cellHeight = 1.0f / (float)screenH * ((float)screenH / worldH);
  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < worldH; y++)
  {
    for (x = 0; x < worldW; x++)
    {
      if (GetCellState(x, y, graphicsContext->pWorldRenderBuffer))
      {
        DrawRect(graphicsContext->pCellDrawData, 
          ((float)x / worldW), ((float)y / worldH), 0.0f,
          cellWidth, cellHeight);
      }
    }
  }

  DrawQuadData(graphicsContext->pCellDrawData);
  ClearQuadDrawData(graphicsContext->pCellDrawData);
  SDL_GL_SwapWindow(window);
}
