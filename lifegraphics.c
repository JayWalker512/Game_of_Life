#include "lifegraphics.h"
#include <SDL2/SDL.h>

LifeGraphicsContext_t *CreateLifeGraphicsContext(LifeWorldDim_t w, LifeWorldDim_t h,
    const char *vertexShaderPath, const char *fragmentShaderPath)
{
  LifeGraphicsContext_t *context = malloc(sizeof(LifeGraphicsContext_t));

  context->pWorldRenderBuffer = NewLifeWorld(w, h);
  context->pQuadDrawData = NewQuadDataBuffer(w * h);
  if (!SetQuadShader(context->pQuadDrawData, 
    BuildShaderProgram(vertexShaderPath, fragmentShaderPath)))
  {
    printf("Couldn't build quad shader(s)!\n");
    DestroyLifeGraphicsContext(context);
    return NULL;
  }

  return context;
}

void DestroyLifeGraphicsContext(LifeGraphicsContext_t *context)
{
  DestroyLifeWorld(context->pWorldRenderBuffer);
  DestroyQuadDrawData(context->pQuadDrawData);
  free(context);
}

void SyncWorldToScreen(SDL_Window *window, ThreadWorldContext_t *worldContext, LifeGraphicsContext_t *graphicsContext, int syncRateHz)
{
  //fair amount of duplicated code here, can be cleaned up?
  static unsigned long endTime = 0;
  if (syncRateHz <= 0)
  {
    SDL_LockMutex(worldContext->lock);
    CopyWorld(graphicsContext->pWorldRenderBuffer, worldContext->front);
    SDL_UnlockMutex(worldContext->lock);

    DrawWorld(window, graphicsContext);
  }
  else if (SDL_GetTicks() > endTime)
  {

    SDL_LockMutex(worldContext->lock);
    CopyWorld(graphicsContext->pWorldRenderBuffer, worldContext->front);
    SDL_UnlockMutex(worldContext->lock);

    DrawWorld(window, graphicsContext);

    unsigned long delayMs = 1000 / syncRateHz;
    endTime = SDL_GetTicks() + delayMs;
  }
}

void DrawWorld(SDL_Window *window, LifeGraphicsContext_t *graphicsContext)
{
  //clear screen first
  glClearColor(0.1, 0.0, 0.0, 0.0);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        DrawRect(graphicsContext->pQuadDrawData, 
          ((float)x / worldW), ((float)y / worldH), 0.0f,
          cellWidth, cellHeight);
      }
    }
  }

  DrawQuadData(graphicsContext->pQuadDrawData, 
    graphicsContext->pQuadDrawData->shader);
  ClearQuadDrawData(graphicsContext->pQuadDrawData);
  SDL_GL_SwapWindow(window);
}
