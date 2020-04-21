#include "benchmark.hpp"
#include "compressonator_codec.hpp"
#include "nvtt_codec.hpp"
#include "directxtex_codec.hpp"

#include <argparse.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <string>

namespace
{
std::string formatBytes(size_t bytes)
{
	static const char* prefixes[] = { "B", "KB", "MB", "GB", "TB" };
	static const size_t prefixCount = sizeof(prefixes) / sizeof(prefixes[0]);

	size_t n = 0;
	auto fbytes = static_cast<float>(bytes);
	while (n < prefixCount - 1 && fbytes >= 1024.f)
	{
		++n;
		fbytes /= 1024.f;
	}

	std::stringstream buffer;
	if (n == 0 || fbytes >= 100.f || fbytes - static_cast<size_t>(fbytes) < 0.05f)
	{
		buffer << static_cast<size_t>(fbytes) << " " << prefixes[n];
	}
	else
	{
		fbytes = std::round(fbytes * 10.f) / 10.f;
		buffer << std::fixed << std::setprecision(1) << fbytes << " " << prefixes[n];
	}

	return buffer.str();
}

struct Parameters
{
	enum class Codec
	{
		Compressonator,
		NVTT,
		DirectXTex,
	};

	std::string inputDir;
	CompressedFormat format;
	CompressionQuality quality;
	Codec codec;
	bool useGPU;
	bool bc7Quick;
	bool bc7Use3Subsets;
};

bool parseFormat(const std::string str, CompressedFormat& format)
{
	if (str == "bc1")
	{
		format = CompressedFormat::BC1;
		return true;
	}
	else if (str == "bc3")
	{
		format = CompressedFormat::BC3;
		return true;
	}
	else if (str == "bc4")
	{
		format = CompressedFormat::BC4;
		return true;
	}
	else if (str == "bc5")
	{
		format = CompressedFormat::BC5;
		return true;
	}
	else if (str == "bc6")
	{
		format = CompressedFormat::BC6;
		return true;
	}
	else if (str == "bc7")
	{
		format = CompressedFormat::BC7;
		return true;
	}
	
	return true;
}

bool parseCodec(const std::string& str, Parameters::Codec& codec)
{
	if (str == "compressonator")
	{
		codec = Parameters::Codec::Compressonator;
		return true;
	}
	else if (str == "nvtt")
	{
		codec = Parameters::Codec::NVTT;
		return true;
	}
	else if (str == "directxtex")
	{
		codec = Parameters::Codec::DirectXTex;
		return true;
	}

	return false;
}

bool parseQuality(const std::string& str, CompressionQuality& quality)
{
	if (str == "low")
	{
		quality = CompressionQuality::Low;
		return true;
	}
	else if (str == "medium")
	{
		quality = CompressionQuality::Medium;
		return true;
	}
	else if (str == "high")
	{
		quality = CompressionQuality::High;
		return true;
	}

	return false;
}

bool parseParameters(int argc, const char* argv[], Parameters& params)
{
	argparse::ArgumentParser parser("Compression implementations benchmark");
	parser.add_argument()
		.name("--input")
		.description("path to a directory with textures to compress")
		.required(true);
	parser.add_argument()
		.name("--format")
		.description("compression format [bc1, bc3, bc4, bc5, bc6, bc7]")
		.required(true);
	parser.add_argument()
		.name("--codec")
		.description("compressor implementation [compressonator, nvtt, directxtex]")
		.required(true);
	parser.add_argument()
		.name("--quality")
		.description("compression quality [low, medium, high]");
	parser.add_argument()
		.name("--gpu")
		.description("enable GPU");
	parser.add_argument()
		.name("--bc7quick")
		.description("enable DirectXTex BC7 flag TEX_COMPRESS_BC7_QUICK");
	parser.add_argument()
		.name("--bc7use3subsets")
		.description("enable DirectXTex BC7 flag TEX_COMPRESS_BC7_USE_3SUBSETS");

	if (auto err = parser.parse(argc, argv))
	{
		std::cerr << err << std::endl;
		parser.print_help();
		return false;
	}

	params.inputDir = parser.get<std::string>("input");
	
	auto formatStr = parser.get<std::string>("format");
	if (!parseFormat(formatStr, params.format))
	{
		std::cerr << "Unknown format " << formatStr << std::endl;
		return false;
	}

	auto codecStr = parser.get<std::string>("codec");
	if (!parseCodec(codecStr, params.codec))
	{
		std::cerr << "Unknown codec " << codecStr << std::endl;
		return false;
	}

	if (parser.exists("quality"))
	{
		auto qualityStr = parser.get<std::string>("quality");
		if (!parseQuality(qualityStr, params.quality))
		{
			std::cerr << "Unknown quality " << qualityStr << std::endl;
			return false;
		}
	}
	else
	{
		params.quality = CompressionQuality::Medium;
	}

	params.useGPU = parser.exists("gpu");
	params.bc7Quick = parser.exists("bc7quick");
	params.bc7Use3Subsets = parser.exists("bc7use3subsets");

	return true;
}

std::unique_ptr<Codec> makeCodec(const Parameters& params)
{
	switch (params.codec)
	{
	case Parameters::Codec::Compressonator:
		return std::make_unique<CompressonatorCodec>();
	
	case Parameters::Codec::NVTT:
	{
		auto codec = std::make_unique<NvttCodec>();
		codec->setCudaEnabled(params.useGPU);
		return codec;
	}
	
	case Parameters::Codec::DirectXTex:
	{
		auto mode = params.useGPU ?
			DirectXTexCodec::Mode::CPU_GPU :
			DirectXTexCodec::Mode::CPU_ONLY;
		auto codec = std::make_unique<DirectXTexCodec>(mode);
		codec->setBC7Quick(params.bc7Quick);
		codec->setBC7Use3Subsets(params.bc7Use3Subsets);
		return codec;
	}

	default:
		return nullptr;
	}
}
} // namespace

int main(int argc, const char* argv[])
{
	Parameters params;
	if (!parseParameters(argc, argv, params))
	{
		return 1;
	}

	auto codec = makeCodec(params);
	codec->setQuality(params.quality);

	Benchmark benchmark(*codec);
	auto results = benchmark.run(params.inputDir, params.format);

	if (results.hasErrors)
	{
		std::cout << "Benchmark completed with errors!" << std::endl;
	}

	std::cout << "Compressed in " << results.elapsedSeconds << " sec\t\t";
	std::cout << "Throughput " << formatBytes(results.throughputBytesPerSec) << "/sec\t\t";
	std::cout << "Error " << std::fixed << std::setprecision(5) << results.compressionError << std::endl;

	return 0;
}