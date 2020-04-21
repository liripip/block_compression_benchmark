#include "compressonator_codec.hpp"

#include <Compressonator.h>

#include <iostream>

namespace
{
CMP_FORMAT translateFormat(CompressedFormat format)
{
	switch (format)
	{
	case CompressedFormat::BC1: return CMP_FORMAT_BC1;
	case CompressedFormat::BC3: return CMP_FORMAT_BC3;
	case CompressedFormat::BC4: return CMP_FORMAT_BC4;
	case CompressedFormat::BC5: return CMP_FORMAT_BC5;
	case CompressedFormat::BC6: return CMP_FORMAT_BC6H;
	case CompressedFormat::BC7: return CMP_FORMAT_BC7;
	default: return CMP_FORMAT_Unknown;
	}
}
} // namespace

bool CompressonatorCodec::doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output)
{
	CMP_CompressOptions options = { 0 };
	options.dwSize = sizeof(options);
	options.bDisableMultiThreading = 0;

	switch (quality())
	{
	case CompressionQuality::Low:
		options.fquality = 0.05f;
		options.nCompressionSpeed = CMP_Speed_SuperFast;
		break;
	case CompressionQuality::Medium:
		options.fquality = 0.5f;
		options.nCompressionSpeed = CMP_Speed_Normal;
		break;
	case CompressionQuality::High:
		options.fquality = 1.0f;
		options.nCompressionSpeed = CMP_Speed_Normal;
		break;
	default:
		break;
	}

	CMP_Texture src;
	src.dwSize = sizeof(src);
	src.dwWidth = input.width;
	src.dwHeight = input.height;
	src.format = CMP_FORMAT_ARGB_8888;
	src.dwPitch = input.width * 4;
	src.dwDataSize = CMP_CalculateBufferSize(&src);
	src.pData = const_cast<unsigned char*>(input.bytes.data());

	CMP_Texture dst;
	dst.dwSize = sizeof(dst);
	dst.dwWidth = input.width;
	dst.dwHeight = input.height;
	dst.format = translateFormat(format);
	dst.dwPitch = 0;
	dst.dwDataSize = CMP_CalculateBufferSize(&dst);

	std::vector<unsigned char> outData(dst.dwDataSize);
	dst.pData = outData.data();

	auto result = CMP_ConvertTexture(&src, &dst, &options, nullptr);
	if (result != CMP_OK)
	{
		std::cerr << "Compressonator error" << std::endl;
		return false;
	}

	output.format = format;
	output.width = input.width;
	output.height = input.height;
	output.bytes = std::move(outData);

	return true;
}
