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

	unsigned long generations = 0;
	while (context->bRunning)
	{
      LifeGeneration(context->back, context->front);
      SwapThreadWorldContextPointers(context);
      generations = DoGensPerSec(generations);

      if (context->bRandomize)
      {
      	pthread_mutex_lock(&context->lock);
      	context->bRandomize = 0;
      	RandomizeWorldStateBinary(context);
      	pthread_mutex_unlock(&context->lock);
      }
	
	}

	return NULL;
}
