#include "benchmark.hpp"

#include <png_utils.hpp>

#include <chrono>
#include <filesystem>
#include <iostream>

namespace
{
bool endsWith(std::string const& str, std::string const& ending)
{
	if (str.length() >= ending.length())
	{
		return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
	}
	else
	{
		return false;
	}
}

size_t relevantChannels(CompressedFormat format)
{
	switch (format)
	{
	case CompressedFormat::BC1: return 3;
	case CompressedFormat::BC3: return 3;
	case CompressedFormat::BC4: return 1;
	case CompressedFormat::BC5: return 2;
	case CompressedFormat::BC6: return 3;
	case CompressedFormat::BC7: return 3;
	default: return 4;
	}
}

UncompressedImage makeImageFromPngReadback(const PngUtils::Readback& readback)
{
	UncompressedImage image;
	image.format = UncompressedFormat::RGBA8;
	image.width = readback.width;
	image.height = readback.height;

	auto size = image.width * image.height * 4;
	image.bytes.reserve(size);

	auto begin = readback.data.get();
	auto end = begin + size;
	std::copy(begin, end, std::back_inserter(image.bytes));

	return image;
}

std::vector<UncompressedImage> loadDataSet(const std::string& dir)
{
	std::vector<PngUtils::Readback> readbacks;
	for (const auto& entry : std::filesystem::directory_iterator(dir))
	{
		const auto& path = entry.path().string();
		if (endsWith(path, ".png"))
		{
			auto readback = PngUtils::readPng(path.c_str());
			if (readback.data == nullptr)
			{
				std::cerr << "Failed to load image" << path << std::endl;
			}
			else if (readback.format != PngUtils::Format::RGBA)
			{
				std::cerr << "Image has unsupported format " << path << std::endl;
			}
			else
			{
				readbacks.push_back(std::move(readback));
			}
		}
	}

	std::vector<UncompressedImage> result;
	result.reserve(readbacks.size());

	std::transform(
		std::begin(readbacks), std::end(readbacks),
		std::back_inserter(result), &makeImageFromPngReadback);

	return result;
}

class ErrorCalculator
{
public:
	ErrorCalculator(size_t relevantChannels)
		: m_relevantChannels(relevantChannels)
	{}

	void addSamples(const UncompressedImage& image0, const UncompressedImage& image1)
	{
		for (size_t pixel = 0, n = image0.bytes.size(); pixel < n; pixel += 4)
		{
			for (size_t channel = 0; channel < m_relevantChannels; ++channel)
			{
				auto s0 = static_cast<double>(image0.bytes[pixel + channel]);
				auto s1 = static_cast<double>(image1.bytes[pixel + channel]);
				auto error = (s1 - s0) / 255.0;
				m_squareErrorSum += error * error;
			}
		}

		m_sampleCount += image0.bytes.size();
	}

	double calculateError() const
	{
		return (m_sampleCount > 0 ? sqrt(m_squareErrorSum / m_sampleCount) : 0.0);
	}

private:
	double m_squareErrorSum = 0.0;
	size_t m_sampleCount = 0;
	const size_t m_relevantChannels;
};
} // namespace

Benchmark::Results Benchmark::run(const std::string& contentDir, CompressedFormat format)
{
	auto uncompressedImages = loadDataSet(contentDir);

	Results results;
	results.hasErrors = false;
	results.processedBytes = 0;
	results.compressionError = 0.0f;

	std::vector<CompressedImage> compressedImages;
	compressedImages.reserve(uncompressedImages.size());

	auto start = std::chrono::steady_clock::now();
	for (const auto& uncompressed : uncompressedImages)
	{
		CompressedImage compressed;
		if (m_codec.compress(uncompressed, format, compressed))
		{
			results.processedBytes += uncompressed.bytes.size();
		}
		else
		{
			std::cerr << "Failed to compress image" << std::endl;
			results.hasErrors = true;
			compressed.bytes.clear();
		}

		compressedImages.push_back(std::move(compressed));
	}
	auto end = std::chrono::steady_clock::now();

	ErrorCalculator calculator(relevantChannels(format));
	for (size_t i = 0, n = uncompressedImages.size(); i < n; ++i)
	{
		const auto& uncompressed = uncompressedImages[i];
		const auto& compressed = compressedImages[i];

		if (compressed.bytes.empty())
		{
			continue;
		}

		UncompressedImage decompressed;
		if (!genericDecompress(compressed, UncompressedFormat::RGBA8, decompressed))
		{
			std::cerr << "Failed to decompress image" << std::endl;
			results.hasErrors = true;
		}
		else if (uncompressed.bytes.size() != decompressed.bytes.size())
		{
			std::cerr << "Image has a different size after the decompression" << std::endl;
			results.hasErrors = true;
		}
		else
		{
			calculator.addSamples(uncompressed, decompressed);
		}
	}

	results.elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

	auto throughput = static_cast<float>(results.processedBytes) / results.elapsedSeconds;
	results.throughputBytesPerSec = static_cast<size_t>(throughput);

	results.compressionError = calculator.calculateError();

	return results;
}
