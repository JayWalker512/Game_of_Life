#include "loadfile.h"
#include "threadlife.h"
#include <stdio.h>

char LoadLifeWorld(LifeWorldBuffer_t *dest, const char *file, char bCenter)
{
	FILE *fp;
	fp = fopen(file, "r");

	if (fp == NULL)
	{
		printf("Couldn't open: %s\n", file);
		return 0;
	}

	LifeWorldDim_t x, y;
	if (bCenter == 1)
	{	
		x = dest->width / 2;
		y = dest->height / 2;
	}
	else
	{
		x = 0;
		y = 0;
	}

	int c = 0;
	printf("Loading file: %s\n", file);
	while ((c = fgetc(fp)) != EOF)
	{
		switch ( c )
		{
			case '.':
				SetCellState(x, y, dest, 0);
				printf(".");
				x++;
				break;
			case 'O':
				SetCellState(x, y, dest, 1);
				printf("O");
				x++;
				break;
			case '\n':
				printf("\n");
				y++;
				if (bCenter)
					x = dest->width / 2;
				else
					x = 0;
				break;
			case '!': //this signifies a comment in the file, ignore it.
				while ((c = fgetc(fp)) != '\n'); //loop to end of line
				break;
			default:
				break;
		}
	}
	printf("\n");

	fclose(fp);
	return 1;
}
