typedef char LifeWorldCell_t;

typedef struct LifeWorld_s {
	LifeWorldCell_t *world;
	long width;
	long height;
} LifeWorld_t;

SDL_Surface *CreateWindow(int width, int height, const char *title);
char CheckInput(void);
LifeWorld_t *NewLifeWorld(long width, long height);
void DestroyLifeWorld(LifeWorld_t *world);
void SwapWorldPointers(LifeWorld_t **front, LifeWorld_t **back);

LifeWorldCell_t GetCellState(long x, long y, LifeWorld_t *world);
void SetCellState(long x, long y, LifeWorld_t *world, LifeWorldCell_t);

LifeWorldCell_t SetWorldState(LifeWorld_t *world, LifeWorldCell_t state);
void RandomizeWorldStateBinary(LifeWorld_t *world, long seed);
void LifeGeneration(LifeWorld_t *newWorld, LifeWorld_t *const oldWorld);
void DrawWorld(SDL_Surface *screen, LifeWorld_t *world);
void SyncWorldToScreen(SDL_Surface *screen, LifeWorld_t *world, int syncRateHz);
unsigned long DoGensPerSec(unsigned long gens);