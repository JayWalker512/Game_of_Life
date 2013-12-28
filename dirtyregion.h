#ifndef DIRTYRECT_H
#define DIRTYRECT_H

typedef char DirtyBufferType_t;
typedef struct DirtyRegionBuffer_s {
	DirtyBufferType_t *buffer;
	int regionSquareDims; //dims in original area units. Ex, a region may be 8x8 src units. Src dims must be multiples of this.
	int numRegions;
	int srcWidth; //dimensions of the original area being represented by regions.
	int srcHeight; //ex a 32x24 (src) area may be represented by 12, 8x8 regions.
} DirtyRegionBuffer_t;

/* srcWidth and srcHeight must be multiples of regionSize. If they aren't, they'll be scaled to the next
highest value that is. Keep this in mind, as forgetting may lead to surprising results in your
code when you may expect a coord to wrap around, and it doesn't because edge dimensions have been
scaled up. */
DirtyRegionBuffer_t *NewDirtyRegionBuffer(const int regionSquareDims, const int srcWidth, const int srcHeight);
void DestroyDirtyRegionBuffer(DirtyRegionBuffer_t *buffer);

/* This function marks a region. These "DirtyRegionBuffers" are really just subdivided
areas from a larger whole. A 16x16 map may have a DirtyRegionBuffer consisting of 4 regions, 
each being 8x8 in size. To mark the upper-left region, we pass in any coordinate from the
upper-left 8x8 quadrant. */  
int MarkDirtyRegion(DirtyRegionBuffer_t *buffer, const int x, const int y);

//clears region values to 0
void ClearDirtyRegionBuffer(DirtyRegionBuffer_t *buffer, int state);

//returns the number of regions from the subdivided source area
int NumRegions(DirtyRegionBuffer_t *buffer);

//returns the value of a given region
int GetRegionValue(DirtyRegionBuffer_t *buffer, const int region);

//copies the upper left most coordinate and dimensions of the region specified to
//the parameters passed in.
int GetRegionSourceDims(const DirtyRegionBuffer_t *buffer, const int region, int *x, int *y, int *w, int *h);

#endif

