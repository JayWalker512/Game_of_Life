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
  graphicsContext->pBackgroundDrawData = NewQuadDataBuffer(1);

  Vector3Set(&graphicsContext->scale, 1.0, 1.0, 1.0);
  Vector3Set(&graphicsContext->translation, 0.0, 0.0, 0.0);
  graphicsContext->bDrawRegions = 0;

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
  DestroyQuadDrawData(context->pBackgroundDrawData);
  free(context);
}

void GetGameGraphicsTranslation(Vector3_t *translation, LifeGameGraphicsContext_t *const context)
{
  Vector3Set(translation, context->translation.x, context->translation.y, context->translation.z);
}

void SetGameGraphicsTranslation(LifeGameGraphicsContext_t *context, Vector3_t *const translation)
{
  Vector3Set(&context->translation, translation->x, translation->y, translation->z);
  SetQuadTranslation(context->pCellDrawData, &context->translation);
  SetQuadTranslation(context->pRegionDrawData, &context->translation);
  SetQuadTranslation(context->pBackgroundDrawData, &context->translation);
}

void GetGameGraphicsScale(Vector3_t *scale, LifeGameGraphicsContext_t *const context)
{
  Vector3Set(scale, context->scale.x, context->scale.y, context->scale.z);
}

void SetGameGraphicsScale(LifeGameGraphicsContext_t *context, Vector3_t *const scale)
{
  Vector3Set(&context->scale, scale->x, scale->y, scale->z);
  SetQuadScale(context->pCellDrawData, &context->scale);
  SetQuadScale(context->pRegionDrawData, &context->scale);
  SetQuadScale(context->pBackgroundDrawData, &context->scale);
}

void SetCellDrawColor(LifeGameGraphicsContext_t *context, Vector3_t *const rgb)
{
  Vector3_t color;
  Vector3Set(&color, rgb->x, rgb->y, rgb->z);
  SetQuadColor(context->pCellDrawData, rgb);
}

void SetRegionDrawColor(LifeGameGraphicsContext_t *context, Vector3_t *const rgb)
{
  Vector3_t color;
  Vector3Set(&color, rgb->x, rgb->y, rgb->z);
  SetQuadColor(context->pRegionDrawData, rgb);
}

void SetBackgroundDrawColor(LifeGameGraphicsContext_t *context, Vector3_t *const rgb)
{
  Vector3_t color;
  Vector3Set(&color, rgb->x, rgb->y, rgb->z);
  SetQuadColor(context->pBackgroundDrawData, rgb);
}

void DrawRegionsEnabled(LifeGameGraphicsContext_t *context, int enabled)
{
  if (enabled)
    context->bDrawRegions = 1;
  else
    context->bDrawRegions = 0;
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
		//end crititcal section!

    DrawWorld(window, graphicsContext);

		if (syncRateHz > 0)
		{
    	unsigned long delayMs = 1000 / syncRateHz;
    	endTime = SDL_GetTicks() + delayMs;
		}
  }

}

void DrawWorld(SDL_Window *window, LifeGameGraphicsContext_t *graphicsContext)
{
  /*
  SetQuadTranslation(graphicsContext->pCellDrawData, &graphicsContext->translation);
  SetQuadTranslation(graphicsContext->pRegionDrawData, &graphicsContext->translation);
  SetQuadScale(graphicsContext->pCellDrawData, &graphicsContext->scale);
  SetQuadScale(graphicsContext->pRegionDrawData, &graphicsContext->scale);
  */

  ClearScreen(0.0, 0.0, 0.0);
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

  Vector3_t backColor;
  Vector3Set(&backColor, 0.04, 0.00, 0.00);
  SetBackgroundDrawColor(graphicsContext, &backColor);
  DrawRect(graphicsContext->pBackgroundDrawData, 0.0, 0.0, 0.0, 1.0, 1.0);
  DrawQuadData(graphicsContext->pBackgroundDrawData);
  ClearQuadDrawData(graphicsContext->pBackgroundDrawData);

  //region drawing goes here eventually.

  Vector3_t cellColor;
  Vector3Set(&cellColor, 1.0, 0.0, 0.0);
  SetCellDrawColor(graphicsContext, &cellColor);
  DrawQuadData(graphicsContext->pCellDrawData);
  ClearQuadDrawData(graphicsContext->pCellDrawData);
  SDL_GL_SwapWindow(window);
}
