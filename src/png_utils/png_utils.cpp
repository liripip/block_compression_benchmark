#include "png_utils.hpp"

#include <png.h>
#include <cstdio>

namespace PngUtils
{
Readback readPng(const char fileName[])
{
	Readback readback;

	auto* file = fopen(fileName, "rb");
	if (file == nullptr)
	{
		return readback;
	}

	unsigned char header[8];
	if (fread(header, 1, 8, file) != 8)
	{
		fclose(file);
		return readback;
	}

	if (!png_check_sig(header, 8))
	{
		fclose(file);
		return readback;
	}

	auto* png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png == nullptr)
	{
		fclose(file);
		return readback;
	}

	auto* info = png_create_info_struct(png);
	if (info == nullptr)
	{
		png_destroy_read_struct(&png, nullptr, nullptr);
		fclose(file);
		return readback;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, nullptr);
		fclose(file);
		return readback;
	}

	png_init_io(png, file);
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);

	auto width = png_get_image_width(png, info);
	auto height = png_get_image_height(png, info);
	auto colorType = png_get_color_type(png, info);
	auto bitDepth = png_get_bit_depth(png, info);

	png_read_update_info(png, info);

	auto rowLength = png_get_rowbytes(png, info);

	auto data = std::make_unique<unsigned char[]>(rowLength * height);
	for (size_t y = 0; y < height; ++y)
	{
		png_read_row(png, data.get() + y * rowLength, nullptr);
	}

	png_destroy_read_struct(&png, &info, nullptr);
	fclose(file);

	readback.data = std::move(data);
	readback.format = (colorType == PNG_COLOR_TYPE_RGBA ? Format::RGBA : Format::UNKNOWN);
	readback.width = width;
	readback.height = height;
	return readback;
}

bool writePng(const char fileName[], Format format, size_t width, size_t height, unsigned char* data)
{
	auto* file = fopen(fileName, "wb");
	if (file == nullptr)
	{
		return false;
	}

	auto* png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png == nullptr)
	{
		fclose(file);
		return false;
	}

	auto* info = png_create_info_struct(png);
	if (info == nullptr)
	{
		png_destroy_write_struct(&png, nullptr);
		fclose(file);
		return false;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_write_struct(&png, &info);
		fclose(file);
		return false;
	}

	png_init_io(png, file);

	png_set_IHDR(png, info, width, height, 8,
		PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png, info);

	for (size_t y = 0; y < height; ++y)
	{
		png_write_row(png, data + width * y * 4);
	}

	png_write_end(png, nullptr);

	png_destroy_write_struct(&png, &info);
	fclose(file);
	return true;
}
} // namespace PngUtils
