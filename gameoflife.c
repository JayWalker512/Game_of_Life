//Game of Life
//Brandon Foltz
//use this line to compile me:
//gcc gameoflife.c -o gameoflife -lSDL -lSDL_gfx -I/usr/include/SDL
#include <stdlib.h>
#include <math.h>
#include "SDL.h"
#include "SDL_gfxPrimitives.h"
#include "gameoflife.h"

int main(int argc, char **argv)
{
    SDL_Surface *screen = CreateWindow(640, 480, "Game of Life");

    LifeWorld_t *pWorldBackBuffer = NewLifeWorld(32, 32);
    LifeWorld_t *pWorldFrontBuffer = NewLifeWorld(32, 32);

    //init an r-pentomino
    SetCellState(6, 7, pWorldFrontBuffer, 1);
    SetCellState(7, 8, pWorldFrontBuffer, 1);
    SetCellState(7, 7, pWorldFrontBuffer, 1);
    SetCellState(7, 6, pWorldFrontBuffer, 1);
    SetCellState(8, 6, pWorldFrontBuffer, 1);

    //and some blocks to make sure the world is toroidal
    SetCellState(0, 0, pWorldFrontBuffer, 1);
    SetCellState(0, 1, pWorldFrontBuffer, 1);
    SetCellState(0, 2, pWorldFrontBuffer, 1);

    char gameRunning = 1;
    unsigned long generations = 0;
    while (gameRunning)
    {
      //Draw world so we can see initial state
      SyncWorldToScreen(screen, pWorldFrontBuffer, 0);
      
      //Simulate a generation from pWorldFrontBuffer
      //and store it in pWorldbackBuffer
      LifeGeneration(pWorldBackBuffer, pWorldFrontBuffer);

      //Swap buffers so pWorldBackBuffer is now pWorldFrontBuffer
      //which we draw next loop
      SwapWorldPointers(&pWorldFrontBuffer, &pWorldBackBuffer);

      generations = DoGensPerSec(generations);
      gameRunning = CheckInput();

      SDL_Delay(100);
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

char CheckInput(void)
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
    }
  }

  return 1;
}

LifeWorld_t *NewLifeWorld(long width, long height)
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

char GetCellState(long x, long y, LifeWorld_t *world)
{
  long wrappedX = abs(x) % world->width;
  long wrappedY = abs(y) % world->height;
  return (world->world[(wrappedY * world->width) + wrappedX]);
}

void SetCellState(long x, long y, LifeWorld_t *world, char state)
{
  world->world[(y * world->width) + x] = state;
}

char NumLiveNeighbors(long x, long y, LifeWorld_t *world)
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

char SetWorldState(LifeWorld_t *world, char state)
{
  long x = 0;
  long y = 0;
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

  long x = 0;
  long y = 0;
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
  long x = 0;
  long y = 0;
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