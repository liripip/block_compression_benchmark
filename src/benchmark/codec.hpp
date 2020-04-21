#pragma once

#include <vector>

enum class CompressionQuality
{
	Low,
	Medium,
	High,
};

enum class UncompressedFormat : size_t
{
	RGBA8,
};

enum class CompressedFormat : size_t
{
	BC1,
	BC3,
	BC4,
	BC5,
	BC6,
	BC7,
};

struct UncompressedImage
{
	UncompressedFormat format;
	size_t width;
	size_t height;
	std::vector<unsigned char> bytes;
};

struct CompressedImage
{
	CompressedFormat format;
	size_t width;
	size_t height;
	std::vector<unsigned char> bytes;
};

class Codec
{
public:
	virtual ~Codec() = default;

	CompressionQuality quality() const { return m_quality; }
	void setQuality(CompressionQuality value) { m_quality = value; }

	bool compress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output);

private:
	virtual bool doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output) = 0;

private:
	CompressionQuality m_quality = CompressionQuality::Medium;
};

bool genericDecompress(const CompressedImage& input, UncompressedFormat format, UncompressedImage& output);