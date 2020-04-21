#pragma once

#include "codec.hpp"

#include <vector>
#include <string>

class Benchmark final
{
public:
	struct Results
	{
		bool hasErrors;
		size_t processedBytes;
		size_t elapsedSeconds;
		size_t throughputBytesPerSec;
		double compressionError;
	};

	Benchmark(Codec& codec) : m_codec(codec) {}

	Results run(const std::string& contentDir, CompressedFormat format);

private:
	Codec& m_codec;
};
