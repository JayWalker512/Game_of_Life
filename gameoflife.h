typedef char LifeWorldCell_t;
typedef long LifeWorldDim_t;

typedef struct LifeWorld_s {
	LifeWorldCell_t *world;
	LifeWorldDim_t width;
	LifeWorldDim_t height;
} LifeWorld_t;

SDL_Surface *CreateWindow(int width, int height, const char *title);
char CheckInput(char *bRandomizeWorld);
LifeWorld_t *NewLifeWorld(LifeWorldDim_t width, LifeWorldDim_t height);
void DestroyLifeWorld(LifeWorld_t *world);
void SwapWorldPointers(LifeWorld_t **front, LifeWorld_t **back);
void CopyWorld(LifeWorld_t *dest, LifeWorld_t * const source);

LifeWorldCell_t GetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world);
void SetCellState(LifeWorldDim_t x, LifeWorldDim_t y, LifeWorld_t *world, LifeWorldCell_t);

LifeWorldCell_t SetWorldState(LifeWorld_t *world, LifeWorldCell_t state);
void RandomizeWorldStateBinary(LifeWorld_t *world, long seed);
void LifeGeneration(LifeWorld_t *newWorld, LifeWorld_t *const oldWorld);
void DrawWorld(SDL_Surface *screen, LifeWorld_t *world);
void SyncWorldToScreen(SDL_Surface *screen, LifeWorld_t *world, int syncRateHz);
unsigned long DoGensPerSec(unsigned long gens);