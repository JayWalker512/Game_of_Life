#ifndef DIRTYRECT_H
#define DIRTYRECT_H

#include "threadlife.h"

typedef struct DirtyBlocksContext_s {
	LifeWorldBuffer_t *current;
	LifeWorldBuffer_t *next;
	unsigned int blockSize; //refers to blocks of (blockSize * blockSize) dimensions.
} DirtyBlocksContext_t;

DirtyBlocksContext_t *NewDirtyBlocksContext(ThreadedLifeContext_t context, unsigned int blockSize);
void DestroyDirtyBlocksContext(DirtyBlocksContext_t *context);

/* This is the "meat". Function marks which blocks need to be simulated next frame. If a cell changes in
anywhere in a block, it will mark that block. If a cell changes on the edge of a block or in a corner
(such that it could affect neighboring blocks simulation), it marks the neighboring blocks that could possibly
be affected as well. */
MarkBlocksDirtyByLifeRules(DirtyBlocksContext_t *context, LifeWorldDim_t x, LifeWorldDim_t y);
SwapDirtyBlocksContextPointers(DirtyBlocksContext_t *context); //uses SwapWorldPointers()


#endif