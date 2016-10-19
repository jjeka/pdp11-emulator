#include "CPngImage.h"

CPngImage CPngImage::Instance;

CPngImage::CPngImage() :
	Data_ (NULL)
{}

CPngImage::~CPngImage()
{
	ASSERT (!Data_);
}

SIVector2D CPngImage::LoadFromFile (const char* FileName, byte** Data)
{
	FILE* File = fopen (FileName, "rb");
	Load_ (File);
	fclose(File);
	
	(*Data) = Data_;

	return Size_;
}

void CPngImage::ValidatePng_ (FILE* File)
{
	png_byte Signature[8] = {};
	fread (Signature, 8, 1, File);
	bool IsPng = !png_sig_cmp (Signature, 0, 8);

	ASSERT(IsPng);
}

void CPngImage::ReadData_ (png_structp Png, png_bytep Data, png_size_t Size)
{
	FILE* Source = (FILE*) png_get_io_ptr(Png);
	ASSERT(fread(Data, Size, 1, Source) == 1);
}

CPngImage::tPngAttr CPngImage::GetPngAttr_ (png_structp Png, png_infop Info)
{
	tPngAttr PngAttr =
	{
		png_get_image_width(Png, Info),
		png_get_image_height(Png, Info),
		png_get_bit_depth(Png, Info),
		png_get_color_type(Png, Info),
		png_get_filter_type(Png, Info),
		png_get_compression_type(Png, Info),
		png_get_interlace_type(Png, Info)
	};
	return PngAttr;
}

void CPngImage::Load_ (FILE* File)
{
	ValidatePng_ (File);
	png_structp Png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!Png)
		error ("Couldn't initialize png read struct");
	png_infop Info = png_create_info_struct (Png);
	if (!Info)
	{
		png_destroy_read_struct(&Png, (png_infopp) 0, (png_infopp) 0);
		error ("Couldn't initialize png info struct");
	}
	png_set_read_fn(Png, (png_voidp) File, ReadData_);
	png_set_sig_bytes(Png, 8);
	png_read_info(Png, Info);

	tPngAttr PngAttr = GetPngAttr_ (Png, Info);

	int Stride = 0;
	if (PngAttr.ColorType == PNG_COLOR_TYPE_RGBA)
	{
		Type_ = CPNGIMAGE_RGBA;
		Stride = PngAttr.Width * 4;
	}
	else if (PngAttr.ColorType == PNG_COLOR_TYPE_RGB)
	{
		Type_ = CPNGIMAGE_RGB;
		Stride = PngAttr.Width * 3;
	}
	else if (PngAttr.ColorType == PNG_COLOR_TYPE_GRAY)
	{
		Type_ = CPNGIMAGE_GRAYSCALE;
		Stride = PngAttr.Width;
	}
	else
		error ("Unknown image type");

	png_bytep* RowPointers = new png_bytep[PngAttr.Height];
	ASSERT (!Data_);
	Data_ = new byte[PngAttr.Height * Stride];

	Size_.x = PngAttr.Width;
	Size_.y = PngAttr.Height;

	for (size_t i = 0; i < PngAttr.Height; i++)
	{
		int Offset = (PngAttr.Height - i - 1) * Stride;
		RowPointers[i] = (png_bytep) Data_ + Offset;
	}
	png_read_image(Png, RowPointers);
	delete[] RowPointers;
	png_destroy_read_struct(&Png, &Info,(png_infopp) 0);
}

void CPngImage::DeleteData()
{
	ASSERT (Data_);
	delete[] Data_;
	Data_ = NULL;
}
