#pragma once

#include "codec.hpp"

#include <memory>

class DirectXTexCodec final : public Codec
{
public:
	enum class Mode { CPU_ONLY, CPU_GPU };

	DirectXTexCodec(Mode mode);
	~DirectXTexCodec() override;

	void setBC7Quick(bool value);
	void setBC7Use3Subsets(bool value);

private:
	bool doCompress(const UncompressedImage& input, CompressedFormat format, CompressedImage& output) override;

private:
	class Impl;
	const std::unique_ptr<Impl> m_impl;
};