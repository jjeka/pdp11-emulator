#include <cstdio>
#include <cinttypes>
#include <cassert>
#include "CPngImage.h"

int main(int argc, const char* argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Three arguments required (image, output file, data name)\n");
		return 1;
	}

	byte* data;
	SIVector2D size = CPngImage::Instance.LoadFromFile(argv[1], &data);

	FILE* file = fopen(argv[2], "wb");
	assert(file);
	fprintf(file, "#ifndef %s_H\n#define %s_H\n\n", argv[3], argv[3]);
	fprintf(file, "const unsigned int %s_SIZE_X = %d;\n", argv[3], size.x);
	fprintf(file, "const unsigned int %s_SIZE_Y = %d;\n\n", argv[3], size.y);
	fprintf(file, "const unsigned char %s_DATA[] = { ", argv[3]);

	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			int c = data[(size.y - 1 - y)*size.x+x];
			fprintf(file, "%d, ", c);
		}
	}

	CPngImage::Instance.DeleteData();
	fprintf(file, "0 };\n\n#endif // %s_H", argv[3]);
	fclose(file);

	return 0;
}
