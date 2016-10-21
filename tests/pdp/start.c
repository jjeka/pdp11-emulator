#include "start.h"

void _init_data();

void _start2()
{
	_init_data();
	exec();
}

extern const int* DATA_SECTION_START;
extern const int* DATA_SECTION_RAM_START;
extern const int* DATA_SECTION_SIZE;

void _init_data()
{
	int i;

	for (i = 0; i < ((int) &DATA_SECTION_SIZE) / 2; i++)
	{
		int src = i + ((int) &DATA_SECTION_START);
		int dst = i + ((int) &DATA_SECTION_RAM_START);
		volatile unsigned short* addrSrc = (volatile unsigned short*) src;
		volatile unsigned short* addrDst = (volatile unsigned short*) dst;
		
		(*addrDst) = (*addrSrc);
	}
}
