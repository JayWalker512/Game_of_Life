//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc gameoflife.c -o gameoflife -lSDL -lSDL_gfx -I/usr/include/SDL -Wall
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "SDL.h"
#include "SDL_gfxPrimitives.h"
#include "gameoflife.h"

int main(int argc, char **argv)
{
    SDL_Surface *screen = CreateWindow(800, 600, "Game of Life");

    LifeWorld_t *pWorldBackBuffer = NewLifeWorld(80, 60);
    LifeWorld_t *pWorldFrontBuffer = NewLifeWorld(80, 60);
    LifeWorld_t *pWorldRenderBuffer = NewLifeWorld(80, 60);

    RandomizeWorldStateBinary(pWorldFrontBuffer, SDL_GetTicks());

    char gameRunning = 1;
    unsigned long generations = 0;
    while (gameRunning)
    {
      //Draw world so we can see initial state
      CopyWorld(pWorldRenderBuffer, pWorldFrontBuffer);
      SyncWorldToScreen(screen, pWorldRenderBuffer, 60);
      
      //Simulate a generation from pWorldFrontBuffer
      //and store it in pWorldbackBuffer
      LifeGeneration(pWorldBackBuffer, pWorldFrontBuffer);

      //Swap buffers so pWorldBackBuffer is now pWorldFrontBuffer
      //which we draw next loop
      SwapWorldPointers(&pWorldFrontBuffer, &pWorldBackBuffer);



      generations = DoGensPerSec(generations);

      char bRandomizeWorld = 0;
      gameRunning = CheckInput(&bRandomizeWorld);
      if (bRandomizeWorld)
        RandomizeWorldStateBinary(pWorldFrontBuffer, SDL_GetTicks());
    }

    DestroyLifeWorld(pWorldFrontBuffer);
    DestroyLifeWorld(pWorldBackBuffer);
    SDL_Quit();

    return 0;
}

SDL_Surface *CreateWindow(int width, int height, const char *title)
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(width, height, 0, 
    SDL_HWSURFACE | SDL_DOUBLEBUF );
  SDL_WM_SetCaption(title, 0 );
  return screen;
}

char CheckInput(char *bRandomizeWorld)
{
  SDL_Event event;
  if (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      return 0; //return 0 if we want to quit
    }

    if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_ESCAPE)
        return 0;

      if (event.key.keysym.sym == SDLK_SPACE)
        *bRandomizeWorld = 1;
    }
  }

  return 1;
}

LifeWorld_t *NewLifeWorld(LifeWorldDim_t width, LifeWorldDim_t height)
{
  LifeWorld_t *world = malloc(sizeof(LifeWorld_t));
  world->world = malloc(sizeof(unsigned char) * (width * height));
  world->width = width;
  world->height = height;

  //Initialize world to dead
  unsigned int i = 0;
  for (i = 0; i < (width * height); i++)
    world->world[i] = 0;

  return world;
}

void DestroyLifeWorld(LifeWorld_t *world)
{
  free(world->world);
  free(world);
}

void SwapWorldPointers(LifeWorld_t **front, LifeWorld_t **back)
{
  LifeWorld_t *temp = *front;
  *front = *back;
  *back = temp;
}

void CopyWorld(LifeWorld_t *dest, LifeWorld_t *source)
{
  //when we implement threading, this will have to lock the source & dest
  assert(source->width == dest->width);
  assert(source->height == dest->height);

  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < source->height; y++)
  {
    for (x = 0; x < source->width; x++)
    {
      SetCellState(x, y, dest, GetCellState(x, y, source));
    }
  }
}

LifeWorldCell_t GetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world)
{
  LifeWorldDim_t wrappedX = abs(x) % world->width;
  LifeWorldDim_t wrappedY = abs(y) % world->height;
  return (world->world[(wrappedY * world->width) + wrappedX]);
}

void SetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world, LifeWorldCell_t state)
{
  world->world[(y * world->width) + x] = state;
}

void RandomizeWorldStateBinary(LifeWorld_t *world, long seed)
{
  srand(seed);

  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < world->height; y++)
  {
    for (x = 0; x < world->width; x++)
    {
      SetCellState(x, y, world, 0); //set to zero first so no leftover state
      SetCellState(x, y, world, (LifeWorldCell_t)(rand() % 2));
    }
  }
}

char NumLiveNeighbors(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world)
{
  char numLive = 0;

  //up left
  if (GetCellState(x - 1, y - 1, world))
    numLive++;

  //up
  if (GetCellState(x, y - 1, world))
    numLive++;

  //up-right
  if (GetCellState(x + 1, y - 1, world))
    numLive++;

  //left
  if (GetCellState(x - 1, y, world))
    numLive++;

  //right
  if (GetCellState(x + 1, y, world))
    numLive++;

  //down-left
  if (GetCellState(x - 1, y + 1, world))
    numLive++;

  //down
  if (GetCellState(x, y + 1, world))
    numLive++;

  //down-right
  if (GetCellState(x + 1, y + 1, world))
    numLive++;

  return numLive;
}

LifeWorldCell_t SetWorldState(LifeWorld_t *world, LifeWorldCell_t state)
{
  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < world->height; y++)
  {
    for (x = 0; x < world->width; x++)
    {
      SetCellState(x, y, world, state);
    }
  }

  return state;
}

void LifeGeneration(LifeWorld_t *newWorld, LifeWorld_t *const oldWorld)
{
  SetWorldState(newWorld, 0); //initialize newWorld before we simulate

  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < oldWorld->height; y++)
  {
    for (x = 0; x < oldWorld->width; x++)
    {
      char numNeighbors = NumLiveNeighbors(x, y, oldWorld);
      char cellIsLiving = GetCellState(x, y, oldWorld);
      if (cellIsLiving)
      {
        if (numNeighbors >= 2 && numNeighbors <= 3)
        {
          SetCellState(x, y, newWorld, 1); //cell was alive, stays alive
        } 
        else if (numNeighbors < 2 || numNeighbors > 3)
        {
          SetCellState(x, y, newWorld, 0); //cell was alive, dies
        }
      }
      
      if (!cellIsLiving && numNeighbors == 3) //cell was dead and has 3 live neighbors
      {
        SetCellState(x,y,newWorld,1); //living cell is born
      }
    }
  }
}

void SyncWorldToScreen(SDL_Surface *screen, LifeWorld_t *world, int syncRateHz)
{
  static unsigned long endTime = 0;
  if (syncRateHz <= 0)
  {
    DrawWorld(screen, world);
    SDL_Flip(screen);
  }
  else if (SDL_GetTicks() > endTime)
  {
    unsigned long delayMs = 1000 / syncRateHz;
    endTime = SDL_GetTicks() + delayMs;
    DrawWorld(screen, world);
    SDL_Flip(screen);
  }
}

void DrawWorld(SDL_Surface *screen, LifeWorld_t *world)
{
  //clear screen first
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  int cellWidth = screen->w / world->width;
  int cellHeight = screen->h / world->height;
  LifeWorldDim_t x = 0;
  LifeWorldDim_t y = 0;
  for (y = 0; y < world->height; y++)
  {
    for (x = 0; x < world->width; x++)
    {
      if (GetCellState(x, y, world))
      {
        boxRGBA(screen, 
              x * cellWidth, y * cellHeight, 
              (x * cellWidth) + cellWidth, (y * cellHeight) + cellHeight,
              255, 0, 0, 255);
      }
      else
      {
        boxRGBA(screen, 
              x * cellWidth, y * cellHeight, 
              (x * cellWidth) + cellWidth, (y * cellHeight) + cellHeight,
              0, 0, 0, 255);
      }
    }
  }
}

unsigned long DoGensPerSec(unsigned long gens)
{
  static unsigned long endTime = 0;
  if (SDL_GetTicks() > endTime)
  {
    endTime = SDL_GetTicks() + 1000;
    printf("Gens/s: %ld\n", gens);
    return 0;
  }
  return ++gens;
}