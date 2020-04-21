#pragma once

#include "codec.hpp"

bool decompressImpl(const CompressedImage& input, UncompressedFormat format, UncompressedImage& output);
