#include "directxtex_codec.hpp"

#include <d3d11.h>
#include <wrl/client.h>
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

IDXGIAdapter* selectBestAdapter()
{
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory))))
	{
		return nullptr;
	}

	IDXGIAdapter* bestAdapter = nullptr;
	IDXGIAdapter* currentAdapter = nullptr;
	SIZE_T bestAdapterMemory = 0;

	for (UINT i = 0; factory->EnumAdapters(i, &currentAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC desc;
		if (FAILED(currentAdapter->GetDesc(&desc)))
		{
			continue;
		}

		if (bestAdapter == nullptr || desc.DedicatedVideoMemory > bestAdapterMemory)
		{
			bestAdapter = currentAdapter;
			bestAdapterMemory = desc.DedicatedVideoMemory;
		}
	}

	return bestAdapter;
}
} // namespace

class DirectXTexCodec::Impl
{
public:
	Impl(Mode mode);

	void setBC7Quick(bool value) { m_bc7Quick = value; }
	void setBC7Use3Subsets(bool value) { m_bc7Use3Subsets = value; }

	bool compress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output);

private:
	Mode m_mode;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
	bool m_bc7Quick = false;
	bool m_bc7Use3Subsets = false;
};

DirectXTexCodec::Impl::Impl(Mode mode)
	: m_mode(mode)
{
	if (m_mode == Mode::CPU_GPU)
	{
		auto bestAdapter = selectBestAdapter();
		if (bestAdapter == nullptr)
		{
			std::cerr << "Faild to find suitable IDXGIAdapter" << std::endl;
		}

		auto hr = D3D11CreateDevice(
			bestAdapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&m_device,
			nullptr,
			&m_deviceContext);

		if (FAILED(hr))
		{
			std::cerr << "Failed to create ID3D11Device" << std::endl;
		}
	}
}

bool DirectXTexCodec::Impl::compress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output)
{
	DirectX::Image inImage;
	inImage.width = input.width;
	inImage.height = input.height;
	inImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	inImage.rowPitch = input.width * 4;
	inImage.slicePitch = input.bytes.size();
	inImage.pixels = const_cast<unsigned char*>(input.bytes.data());

	DirectX::ScratchImage outImages;

	DWORD flags = DirectX::TEX_COMPRESS_PARALLEL | DirectX::TEX_COMPRESS_UNIFORM;
	if (m_bc7Quick)
	{
		flags |= DirectX::TEX_COMPRESS_BC7_QUICK;
	}
	if (m_bc7Use3Subsets)
	{
		flags |= DirectX::TEX_COMPRESS_BC7_USE_3SUBSETS;
	}

	HRESULT hr;
	if (m_mode == Mode::CPU_GPU && (format == CompressedFormat::BC6 || format == CompressedFormat::BC7))
	{
		hr = DirectX::Compress(
			m_device.Get(),
			inImage, translateFormat(format),
			flags, DirectX::TEX_THRESHOLD_DEFAULT,
			outImages);
	}
	else
	{
		hr = DirectX::Compress(
			inImage, translateFormat(format),
			flags, DirectX::TEX_THRESHOLD_DEFAULT,
			outImages);
	}

	if (FAILED(hr) || outImages.GetImageCount() == 0)
	{
		std::cerr << "DirectXTex failed" << std::endl;
		return false;
	}

	auto outImage = outImages.GetImages()[0];

	output.format = format;
	output.width = input.width;
	output.height = input.height;
	output.bytes = std::vector<unsigned char>(outImage.pixels, outImage.pixels + outImage.slicePitch);

	return true;
}

DirectXTexCodec::DirectXTexCodec(Mode mode)
	: m_impl(std::make_unique<Impl>(mode))
{
}

DirectXTexCodec::~DirectXTexCodec() = default;

void DirectXTexCodec::setBC7Quick(bool value)
{
	m_impl->setBC7Quick(value);
}

void DirectXTexCodec::setBC7Use3Subsets(bool value)
{
	m_impl->setBC7Use3Subsets(value);
}

bool DirectXTexCodec::doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output)
{
	return m_impl->compress(input, format, output);
}
