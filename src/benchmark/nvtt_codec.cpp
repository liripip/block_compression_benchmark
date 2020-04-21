#include "nvtt_codec.hpp"

#include <nvtt.h>

#include <algorithm>
#include <iostream>

namespace
{
nvtt::Format translateFormat(CompressedFormat format)
{
	switch (format)
	{
	case CompressedFormat::BC1: return nvtt::Format_BC1;
	case CompressedFormat::BC3: return nvtt::Format_BC3;
	case CompressedFormat::BC4: return nvtt::Format_BC4;
	case CompressedFormat::BC5: return nvtt::Format_BC5;
	case CompressedFormat::BC6: return nvtt::Format_BC6;
	case CompressedFormat::BC7: return nvtt::Format_BC7;
	default: return nvtt::Format_Count;
	}
}

std::vector<unsigned char> rgbaToGbra(std::vector<unsigned char> bytes)
{
	for (size_t i = 0, n = bytes.size(); i < n; i += 4)
	{
		std::swap(bytes[i], bytes[i + 2]);
	}

	return bytes;
}
} // namespace

bool NvttCodec::doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output)
{
	struct ErrorHandler : public nvtt::ErrorHandler
	{
		void error(nvtt::Error e) override
		{
			std::cerr << "NVTT error: " << nvtt::errorString(e) << std::endl;
		}
	};

	struct OutputHandler : public nvtt::OutputHandler
	{
		void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {}

		bool writeData(const void* data, int size) override
		{
			auto begin = static_cast<const unsigned char*>(data);
			auto end = begin + size;

			std::copy(begin, end, std::back_inserter(bytes));

			return true;
		}

		void endImage() override {}

		std::vector<unsigned char> bytes;
	};

	ErrorHandler errorHandler;
	OutputHandler outputHandler;

	nvtt::InputOptions inputOptions;
	inputOptions.setTextureLayout(nvtt::TextureType_2D, input.width, input.height);
	inputOptions.setMipmapGeneration(false);
	inputOptions.setFormat(nvtt::InputFormat_BGRA_8UB);

	{
		auto bytes = rgbaToGbra(input.bytes);
		inputOptions.setMipmapData(bytes.data(), input.width, input.height);
	}

	nvtt::CompressionOptions compressionOptions;
	compressionOptions.setFormat(translateFormat(format));

	switch (quality())
	{
	case CompressionQuality::Low:
		compressionOptions.setQuality(nvtt::Quality_Fastest);
		break;
	case CompressionQuality::Medium:
		compressionOptions.setQuality(nvtt::Quality_Normal);
		break;
	case CompressionQuality::High:
		compressionOptions.setQuality(nvtt::Quality_Production);
		break;
	default:
		break;
	}

	nvtt::OutputOptions outputOptions;
	outputOptions.setOutputHeader(false);
	outputOptions.setOutputHandler(&outputHandler);
	outputOptions.setErrorHandler(&errorHandler);

	nvtt::Compressor compressor;
	compressor.enableCudaAcceleration(m_cudaEnabled);

	auto estimated = compressor.estimateSize(inputOptions, compressionOptions);
	outputHandler.bytes.reserve(static_cast<size_t>(estimated));

	if (!compressor.process(inputOptions, compressionOptions, outputOptions))
	{
		return false;
	}

	output.format = format;
	output.width = input.width;
	output.height = input.height;
	output.bytes = std::move(outputHandler.bytes);

	return true;
}
