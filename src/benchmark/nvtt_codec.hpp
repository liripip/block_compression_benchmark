#pragma once

#include "codec.hpp"

class NvttCodec final : public Codec
{
public:
	bool cudaEnabled() const { return m_cudaEnabled; }
	void setCudaEnabled(bool value) { m_cudaEnabled = value; }

private:
	bool doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output) override;

private:
	bool m_cudaEnabled = false;
};