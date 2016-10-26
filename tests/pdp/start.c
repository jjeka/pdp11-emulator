#include "start.h"

void _init_data();

void _start2()
{
	_init_data();
	exec();
}

extern const int DATA_SECTION_START;
extern int DATA_SECTION_RAM_START;
extern const int DATA_SECTION_SIZE;

void _init_data()
{
	int i;

	for (i = 0; i < ((int) &DATA_SECTION_SIZE) / 2; i++)
	{
		i[&DATA_SECTION_RAM_START] = i[&DATA_SECTION_START];
	}
}
