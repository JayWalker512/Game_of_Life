/* Compile me with;
gcc dirtyregion.c -o dirtyregion -Wall -Wextra -pedantic -std=c99 -lm -DTESTCASE
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "dirtyregion.h"

//private static functions. Declared up here so we dont need to prototype them
static int GetRegionFromCoords(DirtyRegionBuffer_t *buffer, const int x, const int y)
{
	int wrappedX = x;
	int wrappedY = y;

	while (wrappedX >= buffer->srcWidth)
		wrappedX -= buffer->srcWidth;

	while (wrappedX < 0)
		wrappedX += buffer->srcWidth;

	while (wrappedY >= buffer->srcHeight)
		wrappedY -= buffer->srcHeight;

	while (wrappedY < 0)
		wrappedY += buffer->srcHeight;

	int widthInRegions = buffer->srcWidth / buffer->regionSquareDims;
	//int heightInRegions = buffer->srcHeight / buffer->regionSquareDims;

	int xRegion = wrappedX / buffer->regionSquareDims;
	int yRegion = wrappedY / buffer->regionSquareDims; //solid!
	return (yRegion * widthInRegions) + xRegion;
}	

//public functions
#ifdef TESTCASE
int main()
{
	DirtyRegionBuffer_t *buf = NewDirtyRegionBuffer(8, 400, 300);
	int region = 1899;
	int x, y, w, h;
	GetRegionSourceDims(buf, region, &x, &y, &w, &h);
	printf("Region %d ULeft src coords (%d,%d) at %dx%d with %d regions. \n",region,x,y,w,h,NumRegions(buf));

	int inX = 17;
	int inY = 7;
	printf("Region from coords (%d,%d) is %d\n", inX, inY, GetRegionFromCoords(buf, inX, inY));
	return 0;
}
#endif

DirtyRegionBuffer_t *NewDirtyRegionBuffer(const int regionSquareDims, const int srcWidth, const int srcHeight)
{
	DirtyRegionBuffer_t *buffer = malloc(sizeof(DirtyRegionBuffer_t));
	
	int scaledSrcWidth = srcWidth;
	int scaledSrcHeight = srcHeight;

	while (scaledSrcWidth % regionSquareDims != 0)
		scaledSrcWidth++;

	while (scaledSrcHeight % regionSquareDims != 0)
		scaledSrcHeight++;

	//printf("Scaled sizes: %d, %d\n", scaledSrcWidth, scaledSrcHeight);

	int widthInRegions = scaledSrcWidth / regionSquareDims;
	int heightInRegions = scaledSrcHeight / regionSquareDims;

	buffer->buffer = malloc(sizeof(DirtyBufferType_t) * (widthInRegions * heightInRegions));

	buffer->srcWidth = scaledSrcWidth;
	buffer->srcHeight = scaledSrcHeight;
	buffer->regionSquareDims = regionSquareDims;
	buffer->numRegions = widthInRegions * heightInRegions;
	return buffer;
}

void DestroyDirtyRegionBuffer(DirtyRegionBuffer_t *buffer)
{
	free(buffer->buffer);
	free(buffer);
	buffer = NULL;
}

int MarkRegion(DirtyRegionBuffer_t *buffer, const int x, const int y, const int state)
{
	buffer->buffer[GetRegionFromCoords(buffer, x, y)] = state;
	return 1;
}

//clears region values to 0
void ClearDirtyRegionBuffer(DirtyRegionBuffer_t *buffer, int state)
{
	for (int i = 0; i < buffer->numRegions; i++)
		buffer->buffer[i] = state;
}

//returns the number of regions from the subdivided source area
int NumRegions(DirtyRegionBuffer_t *buffer)
{
	return buffer->numRegions;
}

//returns the value of a given region
int GetRegionValue(DirtyRegionBuffer_t *buffer, const int region)
{
	return buffer->buffer[region];
}

//copies the upper left most coordinate and dimensions of the region specified to
//the parameters passed in.
int GetRegionSourceDims(const DirtyRegionBuffer_t *buffer, const int region, int *x, int *y, int *w, int *h)
{
	*w = buffer->regionSquareDims;
	*h = buffer->regionSquareDims;
	int widthInRegions = buffer->srcWidth / buffer->regionSquareDims;
	//int heightInRegions = buffer->srcHeight / buffer->regionSquareDims;
	*x = (region % widthInRegions) * buffer->regionSquareDims; 
	*y = (region / widthInRegions) * buffer->regionSquareDims;

	return region;

	/*
	srcWidth = 32
	srcHeight = 24
	regionSquareDims = 8
	####
	####
	####
	region = 7
	bufferDimX = 4
	bufferDimY = 3
	regionX = region % bufferDimX
	regionY = floor(region / bufferDimY)
	*/
}

