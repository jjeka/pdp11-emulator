#ifndef CPNGIMAGE
#define CPNGIMAGE

#include <stdlib.h>
#include <png.h>
#include <assert.h>
#include <stdio.h>

#define ASSERT assert
#define error(a) abort()

enum CPngImageColorType
{
	CPNGIMAGE_RGBA, 
	CPNGIMAGE_RGB, 
	CPNGIMAGE_GRAYSCALE
};

struct SIVector2D
{
	int x;
	int y;
};

typedef unsigned char byte;

class CPngImage
{
private:
	struct tPngAttr
	{
		unsigned Width;
		unsigned Height;
		unsigned BitDepth;
		unsigned ColorType;
		unsigned FilterType;
		unsigned CompressionType;
		unsigned InterlaceType;
	};
	
	byte*		Data_;
	SIVector2D	Size_;
	int			Type_;

	void Load_ (FILE* File);
	void ValidatePng_ (FILE* File);
	tPngAttr GetPngAttr_ (png_structp Png, png_infop Info);
	void LoadTexture_ (unsigned MipMapLevel = 0);
	void Save_ (const char* FileName);

	static void ReadData_ (png_structp Png, png_bytep Data, png_size_t Size);
	
	CPngImage();
	~CPngImage();

public:

	static CPngImage Instance;

	// TODO: byte** -> byte*&?
	SIVector2D LoadFromFile (const char* FileName, byte** Data = NULL);

	void DeleteData();

};

#endif
