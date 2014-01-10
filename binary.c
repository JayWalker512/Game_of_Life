#include "binary.h" //includes stdint.h
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int GetMaskFromStringInt(const char *bitString)
{
	int outMask = 0;
	uint16_t length = strlen(bitString);
	for (uint16_t i = 0; i < length; i++)
	{
		char tmp[2];
		tmp[0] = bitString[i];
		tmp[1] = '\0';
		int bit = strtol(tmp, NULL, 10);
		SetBitInt(&outMask, bit);
	}

	return outMask;
}
