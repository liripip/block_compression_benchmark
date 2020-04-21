#include "codec.hpp"
#include "decompress_impl.hpp"

bool Codec::compress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output)
{
	return doCompress(input, format, output);
}

bool genericDecompress(const CompressedImage& input, UncompressedFormat format, UncompressedImage& output)
{
	return decompressImpl(input, format, output);
}