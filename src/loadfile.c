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

char SaveLifeWorld(const char *file, LifeWorldBuffer_t *source)
{
	FILE *fp;
	fp = fopen(file, "w");

	if (fp == NULL)
	{
		printf("Couldn't open: %s\n", file);
		return 0;
	}
	for (LifeWorldDim_t y = 0; y < source->height; y++)
	{
		for (LifeWorldDim_t x = 0; x < source->width; x++)
		{
			LifeWorldCell_t cell = GetCellState(x, y, source);
			if (cell == 0)
				fputc('.', fp);
			else if (cell == 1)
				fputc('O', fp);
		}
		fputc('\n', fp);
	}

	fclose(fp);
	return 1;
}