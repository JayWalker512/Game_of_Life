#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include "gameoflife.h"

void *ThreadLifeMain(void *worldContext)
{
	ThreadWorldContext_t *context = worldContext;

	printf("World dims are (%ld, %ld)\n", 
		context->front->width,
		context->front->height);

	while (context->bRunning);

	return NULL;
}