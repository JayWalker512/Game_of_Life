#include "loadfile.h"
#include "threadlife.h"
#include <stdio.h>

char LoadLifeWorld(LifeWorldBuffer_t *dest, const char *file)
{
	FILE *fp;
	fp = fopen(file, "r");

	if (fp == NULL)
		return 0;

	LifeWorldDim_t x = 0;
	LifeWorldDim_t y = 0;
	int c = 0;
	puts("Loading file:");
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
				x = 0;
				break;
			case '!': //this signifies a comment in the file, ignore it.
				while ((c = fgetc(fp)) != '\n'); //loop to end of line
				break;
			default:
				break;
		}
	}

	fclose(fp);
	return 1;
}