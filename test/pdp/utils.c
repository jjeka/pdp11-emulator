#include "utils.h"

void sleep(int t)
{
	int i;
	for (i = 0; i < t; i++)
	{
		int j;
		for (j = 0; j < 1000; j++);
	}
}

void* memset(void* s, int c, size_t n)
{
	size_t i;
	char* data = (char*) s;
	for (i = 0; i < n; i++)
		data[i] = c;

	return s;
}
