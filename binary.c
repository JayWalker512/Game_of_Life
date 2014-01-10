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
		//printf("tmp: %s\n", tmp);
		int bit = atoi(tmp); //atoi converts a STRINT not a char. Duh. 
		//that explains the 1024 garbage error. Use strtol eventually.
		//printf("Bit: %d\n", bit);
		SetBitInt(&outMask, bit);
	}

	return outMask;
}