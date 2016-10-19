#include <cstdio>
#include <cinttypes>
#include <cassert>
#include "CPngImage.h"

int main(int argc, const char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "2 args required\n");
		return 1;
	}

	FILE* file = fopen(argv[1], "wb");
	assert(file);
	fprintf(file, "#ifndef FONT_H\n#define FONT_H\n\nconst unsigned char FONT_DATA[] = { ");

	int i = 0;

	byte* data;
	SIVector2D size = CPngImage::Instance.LoadFromFile("font.png", &data);

	for (int i = 0; i < 128; i++)
	{
		int xx = i / 16;
		int yy = i % 16;

		int xs = 3 + xx * 12;
		int ys = 5 + yy * 18;
		int xe = 3 + xx * 12 + 5;
		int ye = 5 + yy * 18 + 8;
		for (int x = xs; x < xe; x++)
		{
			uint8_t info = 0;
			for (int y = ys; y < ye; y++)
			{
				int r = data[0+3*((size.y - y)*size.x+x)];
				int g = data[1+3*((size.y - y)*size.x+x)];
				int b = data[2+3*((size.y - y)*size.x+x)];

				if (r == 0 && g == 0 && b == 0)
				{
					info |= (1 << (y - ys));
				}
			}
			fprintf(file, "%d, ", info);
		}
	}

	CPngImage::Instance.DeleteData();
	fprintf(file, "0 };\n\n #endif // FONT_H");
	fclose(file);

	return 0;
}
