#include "decompress_impl.hpp"

#include <DirectXTex.h>

#include <iostream>

namespace
{
DXGI_FORMAT translateFormat(CompressedFormat format)
{
	switch (format)
	{
	case CompressedFormat::BC1: return DXGI_FORMAT_BC1_UNORM;
	case CompressedFormat::BC3: return DXGI_FORMAT_BC3_UNORM;
	case CompressedFormat::BC4: return DXGI_FORMAT_BC4_UNORM;
	case CompressedFormat::BC5: return DXGI_FORMAT_BC5_UNORM;
	case CompressedFormat::BC6: return DXGI_FORMAT_BC6H_UF16;
	case CompressedFormat::BC7: return DXGI_FORMAT_BC7_UNORM;
	default: return DXGI_FORMAT_UNKNOWN;
	}
}
} // namespace

bool decompressImpl(const CompressedImage& input, UncompressedFormat format, UncompressedImage& output)
{
	DirectX::Image inImage;
	inImage.width = input.width;
	inImage.height = input.height;
	inImage.format = translateFormat(input.format);
	inImage.pixels = const_cast<unsigned char*>(input.bytes.data());
	DirectX::ComputePitch(inImage.format, inImage.width, inImage.height, inImage.rowPitch, inImage.slicePitch);

	DirectX::ScratchImage outImages;
	auto hr = DirectX::Decompress(inImage, DXGI_FORMAT_R8G8B8A8_UNORM, outImages);
	if (FAILED(hr) || outImages.GetImageCount() == 0)
	{
		std::cerr << "DirectXTex failed" << std::endl;
		return false;
	}

	auto outImage = outImages.GetImages()[0];

	output.format = UncompressedFormat::RGBA8;
	output.width = input.width;
	output.height = input.height;
	output.bytes = std::vector<unsigned char>(outImage.pixels, outImage.pixels + outImage.slicePitch);

	return true;
}
