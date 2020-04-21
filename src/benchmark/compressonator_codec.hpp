#pragma once

#include "codec.hpp"

class CompressonatorCodec final : public Codec
{
private:
	bool doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output) override;
};